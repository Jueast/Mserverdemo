#include "mserver_net.hpp"
#include "mpack_message.hpp"
#include "logging.hpp"
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
                    // release the session number when resource itself is relesed;
                });
    sessions_.insert({x, p});
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
            [this](boost::system::error_code& ec){
                if(!ec)
                {
                    create_session()->start();    
                }
                else
                {
                    ERROR("boost asio error: %s", ec.message().c_str());  
                }
            });
}

}

NetworkManager& NetworkManager::getNetMgr()
{
	static NetworkManager mgr;
	return mgr;
}

//TODO NetworkManager

