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
	boost::asio::io_service& get_io_service() {
        return io_service_;
    }
private:
    MDBManager() = default;
    ~MDBManager() = default;
    void do_login(uint32_t uid, std::string username, std::string salt, boost::asio::ip::udp::endpoint ep);
    void do_mount_world(boost::asio::ip::udp::endpoint ep);
    void do_create_user(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    void do_mount_user(uint32_t uid, boost::asio::ip::udp::endpoint ep);
    void do_query(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    void do_modify(MNet::Mpack m, boost::asio::ip::udp::endpoint ep);
    std::string get_player_table_name(uint32_t x);
    std::string get_player_table_name(std::string s);
    void player_query_helper(MDB::MysqlConnPtr conn, std::string table_string,
                              std::set<uint32_t> uids,
                              MNet::States* states);
    void world_query_helper(MDB::MysqlConnPtr conn, std::set<uint32_t> ids,
                            MNet::States* states);
    MDB::MDBConnectionPool pool_;
    std::shared_ptr<MDB::MDBUDPServer> server_ptr_;
    boost::asio::io_service io_service_;
    std::unordered_map<uint32_t, std::string> player_data_dic_;
    std::unordered_map<std::string, uint32_t> rev_player_data_dic_;
    std::unordered_map<uint32_t, std::string> player_table_dic_;
    std::unordered_map<uint32_t, std::string> world_data_dic_;
    std::unordered_map<std::string, uint32_t> rev_world_data_dic_;

};
#endif
