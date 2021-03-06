#ifndef __MSERVER_MDB__
#define __MSERVER_MDB__

#include <string>
#include <boost/asio.hpp>
#include <mysql++/mysql++.h>
#include "mpack_message.hpp"
namespace MDB 
{
typedef std::shared_ptr<mysqlpp::Connection> MysqlConnPtr;
class MDBConnectionPool : mysqlpp::ConnectionPool {
public:
    void init(std::string db, std::string server,
	      std::string user, std::string password,
	      unsigned int soft_max_conns, unsigned int max_idle_time)
    {
		db_ = db;
		server_ = server;
		user_ = user;
		password_ = password;
		soft_max_conns_ = soft_max_conns;
		max_idle_time_ = max_idle_time;	
    }
    ~MDBConnectionPool()
    {
		clear();
    }
    MysqlConnPtr grab(int);
    void release(const mysqlpp::Connection*);
protected:
    mysqlpp::Connection* create();
    void destroy(mysqlpp::Connection* pc);
    unsigned int max_idle_time()
    {
        return max_idle_time_;
    }
private:
    using mysqlpp::ConnectionPool::grab;
    unsigned int conns_in_use_;
    std::string db_, server_, user_, password_;
    // when conn_in_use_ > soft_max_conns, requests to get connection will be suspended. 
    unsigned int soft_max_conns_, max_idle_time_;	
};

using boost::asio::ip::udp;
class MDBUDPServer {
public:
    MDBUDPServer(boost::asio::io_service& io_service, udp::endpoint& ep) : socket_(io_service, ep){
		do_receive();
    }
    ~MDBUDPServer() = default;
    void init(udp::endpoint ep, boost::asio::io_service io_service); 
    void do_receive();
    void deliver(udp::endpoint ep, std::string s);
private:
    void do_write();
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    enum {max_package_length = 8096};
    char read_data_[max_package_length];
    std::deque<std::pair<udp::endpoint, std::string>> write_data_;
};


}

#endif
