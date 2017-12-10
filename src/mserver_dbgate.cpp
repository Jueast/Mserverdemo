#include "mserver_dbgate.hpp"
#include "logging.hpp"
#include "unistd.h"
#include "xml.hpp"
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


void MDBManager::init(const char* filename)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename);
	INFO("MDBManager loaded %s: %s", filename, result.description());
	pugi::xml_node conf = doc.child("configuration");
	std::string db(conf.child("database").child_value());
	std::string server(conf.child("server").child_value());
	std::string user(conf.child("user").child_value());
	std::string password(conf.child("password").child_value());
	unsigned int soft_max_conns = atoi(conf.child("soft_max_conns").child_value());
	unsigned int max_idle_time = atoi(conf.child("soft_max_conns").child_value());

	pool_.init(db, server, user, password, soft_max_conns, max_idle_time);
}





