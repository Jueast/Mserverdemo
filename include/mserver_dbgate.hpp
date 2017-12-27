#ifndef __MSERVER_DBGATE__
#define __MSERVER_DBGATE__
#include <string>
#include <boost/asio.hpp>
#include "mpack_message.hpp"
#include "mserver_mdb.hpp"
class MDBManager {
public:
    void init(const char * filename);
    MDB::MysqlConnPtr grab();
    static MDBManager& getMDBMgr();
    void processRequest(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
private:
    MDBManager() = default;
    ~MDBManager() = default;
    void do_login(uint32_t uid, std::string username, std::string salt, boost::asio::ip::udp::endpoint ep);
    void do_mount_world(boost::asio::ip::udp::endpoint ep);
    void do_create_user(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    void do_mount_user(uint32_t uid, boost::asio::ip::udp::endpoint ep);
    void do_query(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    void do_modify(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    MDB::MDBConnectionPool pool_;
    std::shared_ptr<MDB::MDBUDPServer> server_ptr_;
    std::unordered_map<uint32_t, std::string> player_data_dic_;
    std::unordered_map<std::string, uint32_t> rev_player_data_dic_;
    std::unordered_map<uint32_t, std::string> world_data_dic_;
    std::unordered_map<std::string, uint32_t> rev_world_data_dic_;

};
#endif
