#include "mserver_dbgate.hpp"
#include "logging.hpp"
#include "unistd.h"
namespace MDB
{

MysqlConnPtr MDBConnectionPool::grab(int)
{
    // WARNING!! No explicit exclusive accesss control here, 
	// thus conn_in_use_ could be any positive integer value.
	while(conns_in_use_ > soft_max_conns_){
		sleep(1);
	}
	++conns_in_use_;
	auto p = grab();
	return MysqlConnPtr(p, [this, p]{ this->release(p); });
}

void MDBConnectionPool::release(mysqlpp::Connection* pc)
{
	mysqlpp::ConnectionPool::release(pc);
	--conns_in_use_;
}

inline mysqlpp::Connection* MDBConnectionPool::create()
{
	return new mysqlpp::Connection(
		db_.c_str(),
		server_.c_str(),
		user_.c_str(),
		password_.c_str());
}	

inline void MDBConnectionPool::destroy(mysqlpp::Connection* pc)
{
	delete pc;
}



}
