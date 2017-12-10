#ifndef __MSERVER_DBGATE__
#define __MSERVER_DBGATE__
#include <string>
#include <mysql++/mysql++.h>
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
	void release(mysqlpp::Connection* pc);
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
}
class MDBManager {
public:
    void init(const char * filename);
	MDB::MysqlConnPtr grab();
    static MDBManager& getMDBMgr();
private:
    MDBManager() = default;
    ~MDBManager() = default;
    MDB::MDBConnectionPool pool_;
};


#endif
