#include <boost/asio.hpp>
#include "mpack_message.hpp"
#include "mserver_dbgate.hpp"
#include "logging.hpp"
#include "unistd.h"
#include "xml.hpp"
MDBManager& MDBManager::getMDBMgr()
{
    static MDBManager mgr;
    return mgr;

}
void MDBManager::init(const char* filename)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(filename); 
    INFO("MDBManager loaded %s: %s", filename, result.description());
    pugi::xml_node db_conf = doc.child("configuration").child("db");
    std::string db(db_conf.child("database").child_value());
    std::string server(db_conf.child("server").child_value());
    std::string user(db_conf.child("user").child_value());
    std::string password(db_conf.child("password").child_value());
    unsigned int soft_max_conns = atoi(db_conf.child("soft_max_conns").child_value());
    unsigned int max_idle_time = atoi(db_conf.child("soft_max_conns").child_value());
    pool_.init(db, server, user, password, soft_max_conns, max_idle_time);

    pugi::xml_node game_conf = doc.child("configuration").child("game_file");
    std::string player_file(game_conf.child("player").child_value());
    std::string world_file(game_conf.child("world").child_value());

    pugi::xml_document player_doc, world_doc;
    pugi::xml_parse_result player_result = player_doc.load_file(player_file.c_str());
    INFO("MDBManager loaded %s: %s", player_file.c_str(), player_result.description());
    pugi::xml_parse_result world_result = world_doc.load_file(world_file.c_str());
    INFO("MDBManager loaded %s: %s", world_file.c_str(), world_result.description());
    uint32_t index = 10000;
    for(pugi::xml_node cat : player_doc.child("player").children())
    {
        for(pugi::xml_node col : cat.children())
        {
            index += 1;
            player_data_dic_[index] = col.child_value();
            rev_player_data_dic_[col.child_value()] = index;
        }
        index += 10000;
    }
    index = 10000;
    for(pugi::xml_node cat : world_doc.child("world").children())
    {
        for(pugi::xml_node col : cat.children())
        {
            index += 1;
            world_data_dic_[index] = col.child_value();
            rev_world_data_dic_[col.child_value()] = index;
        }
        index += 10000;

    }
    INFO("Player dic size: %d, World dic size: %d", player_data_dic_.size(), world_data_dic_.size());
    using boost::asio::ip::udp;
    pugi::xml_node udp_conf = doc.child("configuration").child("udp");
    std::string ip(udp_conf.child("ip").child_value());
    std::string port(udp_conf.child("port").child_value());
    udp::endpoint ep(boost::asio::ip::address_v4::from_string(ip.c_str()), atoi(port.c_str())); 
	server_ptr_ = std::make_shared<MDB::MDBUDPServer>(io_service_, ep); 
}

void MDBManager::processRequest(MNet::Mpack m, boost::asio::ip::udp::endpoint ep) 
{
    DEBUGIF(m.type() != MNet::Mpack::CONTROL, "Wrong type udp request arrived.(not control");
    switch(m.control()){
        case(MNet::Mpack::AUTH): 
        {
            uint32_t uid = m.login().uid();
            std::string username = m.login().username();
            std::string salt = m.login().salt();
            io_service_.post([uid, username, salt, ep, this]()
                    {
                        do_login(uid, username, salt, ep);
                    });
            break;
        }
        case(MNet::Mpack::CREATE_USER):
        {
            io_service_.post([this, m, ep](){
                    do_create_user(m, ep);});
            break;

        }
        case(MNet::Mpack::MOUNT_WORLD):
        {
            io_service_.post([this, ep](){
                    do_mount_world(ep);});
            break;
        }
        case(MNet::Mpack::MOUNT_USER):
        {
            uint32_t uid = m.login().uid();
            io_service_.post([this, uid, ep]()
            {
               do_mount_user(uid, ep); 
            });
            break;
        }
        case(MNet::Mpack::SYNC):
        {
            io_service_.post(
                    [this, m, ep]()
                    {   
                        do_sync(m, ep);
                    });
        }
        default:
            break;
    }
}

void MDBManager::do_login(uint32_t uid, std::string username, std::string salt, boost::asio::ip::udp::endpoint ep)
{
    INFO("Authenticating user...uid: %d, username: %s", uid, username.c_str());
    auto conn = pool_.grab(0);
    // check if uid or username exists
    mysqlpp::Query query = conn->query("select * from users_salts where uid = %0:uid OR username = %1q:username ");
    query.parse();
    mysqlpp::StoreQueryResult res = query.store(std::to_string(uid), username);
    bool flag = false;
    if(res && res.num_rows() != 0 && res[0]["salt"] == mysqlpp::String(salt)){
        flag = true;
    }
    
    MNet::Mpack m;
    DEBUGIF(!flag, "Login failed!");
    m.set_type(MNet::Mpack::CONTROL);
    if(!flag)
        m.set_control(MNet::Mpack::ACK_NO);
    else
        m.set_control(MNet::Mpack::ACK_YES);
    server_ptr_->deliver(ep, m.SerializeAsString());
}
using boost::asio::ip::udp;
void MDBManager::do_mount_world(udp::endpoint ep)
{
    INFO("Get world data from database...");
    auto conn = pool_.grab(0);
    // get latest world record
    mysqlpp::Query query = conn->query("SELECT  FROM world where world.ts = ( select max(ts) from world);");
    query.parse();
    mysqlpp::StoreQueryResult res = query.store();
    MNet::Mpack m;
    m.set_type(MNet::Mpack::CONTROL);
    m.set_control(MNet::Mpack::MOUNT_WORLD);
    auto w = m.mutable_states()->mutable_world_attrs();
    for(size_t i = 0; i < res.field_names()->size(); i++) {
        std::string s = res.field_name(i);
        if(s == "ts")
            continue;
        (*w)[rev_player_data_dic_[s]] = res[0].[s.c_str()];
    }

}

void MDBManager::do_create_user(MNet::Mpack m, udp::endpoint ep)
{
}

void MDBManager::do_mount_user(uint32_t uid, udp::endpoint ep)
{
}

void MDBManager::do_query(MNet::Mpack m, udp::endpoint ep)
{
}

void MDBManager::do_sync(MNet::Mpack m, udp::endpoint ep)
{
}

int main(int argc, const char* argv[]){
	try{
		MDBManager& mgr = MDBManager::getMDBMgr();
		Logging::Logger& logger = Logging::Logger::getLogger();
		logger.setFileName("log/dbgate.log");
		logger.setLogLevel(Logging::level::fatal);
		if(argc > 2)
	    {
		    std::cerr << "Usage: dbgate [configure filename ('dbgate.conf' as default)]\n";
			exit(1);
		}
		if(argc == 2)
		{
			mgr.init(argv[1]);
		}
		else
		{
			mgr.init("dbgate.conf");
		}
		mgr.get_io_service().run();
	}
	catch(std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	return 0;
}

