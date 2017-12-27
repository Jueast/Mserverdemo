#include "common.hpp"
#include "mpack_message.hpp"
#include "mserver_net.hpp"
#include "mserver_state.hpp"
#include "logging.hpp"
#include "xml.hpp"
namespace MNet 
{

void NetworkSession::start()
{
    conn_.start();
}


void NetworkSession::dispatch(Mpack m)
{
    if(m.type() == Mpack::HEARTBEAT)
    {
        //TODO update timer;
    }
    // set session_id here.
    m.set_session_id(session_id_);
    // dispatch according to state
    switch(state_) {
        case SESSION_UNAUTHORIZED:
        {
            dispatch_unauthorized(std::move(m));
            break;
        }
        default:
            break;
    }
}

void NetworkSession::deliver(Mpack r)
{
    switch(state_) {
        case SESSION_UNAUTHORIZED:
        {
            deliver_unauthorized(std::move(r));
            break;
        }
        default:
            break;
    }
}

void NetworkSession::dispatch_unauthorized(Mpack m)
{
    switch(m.type()){
        case Mpack::LOGIN:
        {
            NetworkManager::getNetMgr().login(std::move(m));
            break;
        }
        // TODO other type
        default:
            break;
    }
    
}

void NetworkSession::deliver_unauthorized(Mpack r)
{
    switch(r.type())
    {
        case Mpack::CONTROL:
        {
            if(!r.error())
            {
                state_ = SESSION_AUTHORIZED;
                INFO("Session %u was authorized now.", session_id_);
            }
            conn_.deliver(std::move(r));
            break;
        }
        default:
            break;
    }

}
void NetworkSession::close() 
{
    server_.close(session_id_);
}

TCPServer::TCPServer(boost::asio::io_service& io_service,
		  tcp::endpoint& ep,
                  uint32_t max_num_of_sessions)
            :  acceptor_(io_service, ep),
	       socket_(io_service)					
{
    session_nums_.resize(max_num_of_sessions);
    for(uint32_t i = 0; i < session_nums_.size(); i++)
    {
        session_nums_[i] = i+1; // initilize ids of sessions to assign.
    }
    do_accept();
}

TCPServer::NetSessionPtr TCPServer::create_session()
{
    uint32_t x = session_nums_.front();
    session_nums_.pop_front();
    auto p = NetSessionPtr(
                new NetworkSession(std::move(socket_), *this, x),
                    [this, x](NetworkSession* p){
                       release_session(p, x); 
                     //release the session number when resource itself is relesed;
                    });
    sessions_.insert({x, p});
    INFO("Connection get, session created.(id: %u)", x);
    return p;
};

void TCPServer::release_session(NetworkSession * p, uint32_t x) 
{
    session_nums_.push_back(x);
    delete p;   
};

void TCPServer::do_accept()
{
    acceptor_.async_accept(socket_,
            [this](const boost::system::error_code& ec){
                if(!ec)
                {
                    create_session()->start();    
                }
                else
                {
                    ERROR("boost asio error: %s", ec.message().c_str());  
                }
				do_accept();
            });
}

void TCPServer::deliver(Mpack r)
{
    uint32_t session_id = r.session_id();
    sessions_.at(session_id)->deliver(std::move(r));
}

void TCPServer::close(uint32_t session_id)
{
    INFO("Session %u closed.", session_id);
    sessions_.erase(session_id);
}

}

const size_t udp_limit = 10240;
NetworkManager& NetworkManager::getNetMgr()
{
	static NetworkManager mgr;
	return mgr;
}
void NetworkManager::sync(MNet::Mpack m) 
{
	INFO("Sync gameserver data to dbgate..");
	boost::asio::spawn(get_io_service(),
		[this, m](boost::asio::yield_context yield)
		{
			char data[udp_limit];
			m.SerializeToArray(data, udp_limit);
			udp::socket sock(get_io_service());
			sock.open(udp::v4());
			sock.async_send_to(
				boost::asio::buffer(data),
				udp_server_ptr_->get_dbgate_address(),
				yield);
			udp::endpoint from;
			std::size_t l = sock.async_receive_from(boost::asio::buffer(data), from, yield);
			DEBUG("Get udp control message from %s", from.address().to_string().c_str());
            MNet::Mpack r;
            r.ParseFromArray(data, l);
            r.set_session_id(m.session_id());
			StateManager::getStateMgr().addTask(std::move(r));	
		});
}

void NetworkManager::mountWorld()
{
    INFO("Mount world from database...");
    char data[udp_limit];
    MNet::Mpack m;
    m.set_type(MNet::Mpack::CONTROL);
    m.set_control(MNet::Mpack::MOUNT_WORLD);
    m.SerializeToArray(data, udp_limit);
    udp::socket sock(get_io_service());
    sock.open(udp::v4());
    sock.send_to(
        boost::asio::buffer(data),
        udp_server_ptr_->get_dbgate_address());
    udp::endpoint from;
    std::size_t l = sock.receive_from(boost::asio::buffer(data), from);
    DEBUG("Get udp control message from %s", from.address().to_string().c_str());
    MNet::Mpack r;
    r.ParseFromArray(data, l);
    StateManager::getStateMgr().mountWorld(r.world());
}

void NetworkManager::login(MNet::Mpack m) 
{
    INFO("Get login request from session %u", m.session_id());
    m.set_type(MNet::Mpack::CONTROL);
    m.set_control(MNet::Mpack::AUTH);
    boost::asio::spawn(get_io_service(),
        [this, m](boost::asio::yield_context yield){
            char data[40960];
            m.SerializeToArray(data, 40960);
            udp::socket sock(get_io_service());
            sock.open(udp::v4());
            sock.async_send_to(
                    boost::asio::buffer(data),
                    udp_server_ptr_->get_dbgate_address(),
                    yield);
            udp::endpoint from;
            std::size_t l = sock.async_receive_from(boost::asio::buffer(data), from, yield);
            DEBUG("Get udp control message from %s", from.address().to_string().c_str());
            MNet::Mpack r;
            r.ParseFromArray(data, l);
            bool ack = r.control() == MNet::Mpack::ACK_YES;
            r.set_session_id(m.session_id());
	    r.set_type(MNet::Mpack::INFO);
            r.set_error(!ack);
            r.clear_control();
            tcp_server_ptr_->deliver(std::move(r)); 
            if(!ack)
                return;
            MNet::Mpack mount_request;
            mount_request.set_type(MNet::Mpack::CONTROL);
            mount_request.set_control(MNet::Mpack::MOUNT_USER);
            mount_request.mutable_login()->set_uid(m.login().uid());
            mount_request.SerializeToArray(data, 40960);
            sock.async_send_to(
                boost::asio::buffer(data),
                udp_server_ptr_->get_dbgate_address(),
                yield);
            l = sock.async_receive_from(boost::asio::buffer(data), from, yield);
            r.Clear();
            r.ParseFromArray(data, l);    
            uint32_t uid = m.login().uid();
            StateManager::getStateMgr().mountPlayer(uid, std::move(r.players().at(uid))); 
        }); 
            
}
void NetworkManager::deliver(MNet::Mpack m)
{
    tcp_server_ptr_->deliver(std::move(m));
}
void NetworkManager::init(const char* filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename); 
    INFO("NetworkManager loaded %s: %s", filename, result.description());
    using boost::asio::ip::tcp;
    pugi::xml_node db_conf = doc.child("configuration").child("tcp");
    std::string t_ip(db_conf.child("ip").child_value());
    std::string t_port(db_conf.child("port").child_value());
    std::string max_num_of_sessions(db_conf.child("max_num_of_sessions").child_value());
    tcp::endpoint t_ep(boost::asio::ip::address_v4::from_string(t_ip.c_str()), atoi(t_port.c_str()));
    uint32_t max_num = atoi(max_num_of_sessions.c_str());

    using boost::asio::ip::udp;
    pugi::xml_node udp_conf = doc.child("configuration").child("udp");
    std::string u_ip(udp_conf.child("ip").child_value());
    std::string u_port(udp_conf.child("port").child_value());
    udp::endpoint u_ep(boost::asio::ip::address_v4::from_string(u_ip.c_str()), atoi(u_port.c_str())); 

    pugi::xml_node dbgate_conf = doc.child("configuration").child("dbgate");
    std::string db_ip(dbgate_conf.child("ip").child_value());
    std::string db_port(dbgate_conf.child("port").child_value());
    udp::endpoint db_ep(boost::asio::ip::address_v4::from_string(db_ip.c_str()), atoi(db_port.c_str()));

    udp_server_ptr_ = std::make_shared<MNet::UDPServer>(get_io_service(), u_ep, db_ep);
    mountWorld();

    tcp_server_ptr_ = std::make_shared<MNet::TCPServer>(get_io_service(), t_ep, max_num);
    INFO("NetworkManager was initialized now.");
}
