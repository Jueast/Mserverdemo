#include <ctime>
#include <boost/asio.hpp>
#include "mpack_message.hpp"
#include "mserver_dbgate.hpp"
#include "mserver_dbgate_sqlss.h"
#include "common.hpp"
#include "logging.hpp"
#include "unistd.h"
#include "xml.hpp"

static const uint32_t index_limit = 10000;
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
    uint32_t index = index_limit;
    for(pugi::xml_node cat : player_doc.child("player").children())
    {
        for(pugi::xml_node col : cat.children())
        {
            index += 1;
            player_data_dic_[index] = col.child_value();
            rev_player_data_dic_[col.child_value()] = index;
        }
        index += index_limit;
    }
    index = index_limit;
    for(pugi::xml_node cat : world_doc.child("world").children())
    {
        for(pugi::xml_node col : cat.children())
        {
            index += 1;
            world_data_dic_[index] = col.child_value();
            rev_world_data_dic_[col.child_value()] = index;
        }
        index += index_limit;

    }
    INFO("Player dic size: %d, World dic size: %d", player_data_dic_.size(), world_data_dic_.size());
    using boost::asio::ip::udp;
    pugi::xml_node udp_conf = doc.child("configuration").child("udp");
    std::string ip(udp_conf.child("ip").child_value());
    std::string port(udp_conf.child("port").child_value());
    udp::endpoint ep(boost::asio::ip::address_v4::from_string(ip.c_str()), atoi(port.c_str())); 
    server_ptr_ = std::make_shared<MDB::MDBUDPServer>(get_io_service(), ep); 
}

void MDBManager::processRequest(MNet::Mpack m, boost::asio::ip::udp::endpoint ep) 
{
    DEBUGIF(m.type() != MNet::Mpack::CONTROL, "Wrong type udp request arrived.(not control)");
    switch(m.control()){
        case(MNet::Mpack::AUTH): 
        {
            uint32_t uid = m.login().uid();
            std::string username = m.login().username();
            std::string salt = m.login().salt();
            get_io_service().post([uid, username, salt, ep, this]()
                    {
                        do_login(uid, username, salt, ep);
                    });
            break;
        }
        case(MNet::Mpack::CREATE_USER):
        {
            get_io_service().post([this, m, ep](){
                    do_create_user(m, ep);});
            break;

        }
        case(MNet::Mpack::MOUNT_WORLD):
        {
            get_io_service().post([this, ep](){
                    do_mount_world(ep);});
            break;
        }
        case(MNet::Mpack::MOUNT_USER):
        {
            uint32_t uid = m.login().uid();
            get_io_service().post([this, uid, ep]()
            {
               do_mount_user(uid, ep); 
            });
            break;
        }
        case(MNet::Mpack::SYNC):
        {
            get_io_service().post(
                    [this, m, ep]()
                    {   
                        do_modify(m, ep);
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
    MNet::Mpack m;
    m.set_type(MNet::Mpack::CONTROL);
    m.set_control(MNet::Mpack::MOUNT_WORLD);
    auto conn = pool_.grab(0);
    mysqlpp::Query query = conn->query("select * from world where ts=(select max(ts) from world)");
    mysqlpp::StoreQueryResult res = query.store();
    if(res.empty()){
        m.mutable_world(); // set empty world;
        INFO("Get empty world data."); 
    }
    else{
        world world_now = res[0];
        INFO("Get world data at %s.", world_now.ts.str().c_str()); 
        MNet::World w;
        w.ParseFromArray(world_now.dat.c_str(), world_now.dat.length());
       (*m.mutable_world()) = w;
    }
    server_ptr_->deliver(ep, m.SerializeAsString());
}

void MDBManager::do_create_user(MNet::Mpack m, udp::endpoint ep)
{
    INFO("Create user %u now...", m.login().uid());   
    uint32_t uid = m.login().uid();
    std::string username = m.login().username();
    std::string salt = m.login().salt();
    {
        auto conn = pool_.grab(0);
        mysqlpp::Query query = conn->query("select uid from player where uid=%0:uid");
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(uid);
        if(!res.empty()){
            ERROR("Player %u exists!", uid);
            MNet::Mpack a;
            a.set_type(MNet::Mpack::CONTROL);
            a.set_control(MNet::Mpack::ACK_NO);
            server_ptr_->deliver(ep, a.SerializeAsString());
            return;
        }
        mysqlpp::Transaction trans(*conn,
            mysqlpp::Transaction::serializable,
            mysqlpp::Transaction::session);
        users_salts salt_row(uid, username, salt);
        MNet::Player p;
        player char_row(uid, mysqlpp::DateTime::now(), mysqlpp::String(p.SerializeAsString()));
        query.insert(salt_row);
        query.execute();
        query.insert(char_row);
        query.execute();
        trans.commit();
    }
    MNet::Mpack a;
    a.set_type(MNet::Mpack::CONTROL);
    a.set_control(MNet::Mpack::ACK_YES);
    server_ptr_->deliver(ep, a.SerializeAsString());

}

void MDBManager::do_mount_user(uint32_t uid, udp::endpoint ep)
{
    INFO("Mount user %u now...", uid);
    MNet::Mpack m;
    {   
        auto conn = pool_.grab(0);
        mysqlpp::Query query = conn->query("select * from player where uid = %0:uid");
        query.parse();
        mysqlpp::StoreQueryResult res = query.store(uid);
        if(res.empty())
            ERROR("No user %u!", uid);
        else
        {
            player p = res[0];
            INFO("Get player %u data at %s", p.uid, p.ts.str().c_str());
            MNet::Player mp;
            mp.ParseFromArray(p.dat.c_str(), p.dat.length());
            (*m.mutable_players())[uid] = mp;
        }
    }
    m.set_type(MNet::Mpack::CONTROL);
    m.set_control(MNet::Mpack::MOUNT_USER);
    server_ptr_->deliver(ep, m.SerializeAsString());
    
}

// no pratical use now.
void MDBManager::do_query(MNet::Mpack m, udp::endpoint ep)
{

}
void MDBManager::do_modify(MNet::Mpack m, udp::endpoint ep)
{
    INFO("Modifying data now...");
    auto conn = pool_.grab(0);
    {
        mysqlpp::Query query = conn->query("select * from world where ts = (select max(ts) from world)");
        mysqlpp::StoreQueryResult res = query.store();
        if(res.empty()){
            world world_now(mysqlpp::DateTime::now(), mysqlpp::String(m.world().SerializeAsString()));
            query = conn->query();
            query.insert(world_now);
        }
        else
        {   
            world world_now = res[0];
            MNet::World w;
            w.ParseFromArray(world_now.dat.c_str(), world_now.dat.length());
            update_world(w, m.world());       
            world_now.dat = mysqlpp::String(w.SerializeAsString());
            world_now.ts = mysqlpp::DateTime::now();
            query = conn->query();
            query.insert(world_now);
        }
        query.execute();
    }

    {
        for(auto it = m.players().begin(); it != m.players().end();it++)
        {
            uint32_t uid = it->first;
            mysqlpp::Query query = conn->query("select * from player where uid = %0:uid");
            query.parse();
            mysqlpp::StoreQueryResult res = query.store(uid);
            if(res.empty()){
                ERROR("No user %u!", uid);
                continue;
            }
            else
            {
                player p = res[0];
                auto p_old = p;
                MNet::Player mp;
                mp.ParseFromArray(p_old.dat.c_str(), p_old.dat.length());
                update_player(mp, it->second);
                p.dat = mysqlpp::String(mp.SerializeAsString());
                p.ts = mysqlpp::DateTime::now();
                query.update(p_old, p);
                query.execute();
            }

        }
    }
    MNet::Mpack a;
    a.set_type(MNet::Mpack::CONTROL);
    a.set_control(MNet::Mpack::ACK_YES);
    server_ptr_->deliver(ep, a.SerializeAsString());
    
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
	get_io_service().run();
    }
    catch(std::exception& e)
    {
    	std::cerr << e.what() << std::endl;
    }
    return 0;
}

