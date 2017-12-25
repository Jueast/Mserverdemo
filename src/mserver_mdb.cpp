#include <boost/asio.hpp>
#include "mpack_message.hpp"
#include "mserver_dbgate.hpp"
#include "logging.hpp"
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
	return MysqlConnPtr(p, [this](mysqlpp::Connection* p){ this->release(p); });
}

void MDBConnectionPool::release(const mysqlpp::Connection* pc)
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
using boost::asio::ip::udp;

void MDBUDPServer::do_receive(){
    socket_.async_receive_from(
            boost::asio::buffer(read_data_), sender_endpoint_,
            [this](boost::system::error_code ec, std::size_t bytes_recvd){
                if(!ec && bytes_recvd > 0)
                {
                    INFO("Receive message from %s", sender_endpoint_.address().to_string().c_str());
					MNet::Mpack m;
                    m.ParsePartialFromArray(read_data_, bytes_recvd); 
                    MDBManager::getMDBMgr().processRequest(std::move(m), sender_endpoint_);
                    do_receive();
                }
                else
                {
                   if(!ec)
                   {
                        ERROR("boost asio error: %s", ec.message().c_str());
                   }
                   else
                   {
                        WARN("0 length message from %s", sender_endpoint_.address().to_string().c_str());
                   }
                }

            });
}

void MDBUDPServer::deliver(udp::endpoint ep, std::string s)
{
    bool write_in_process = !write_data_.empty();
    write_data_.emplace_back(ep, s);
    if(!write_in_process)
    {
        do_write();
    }
    
}

void MDBUDPServer::do_write()
{
    socket_.async_send_to(
            boost::asio::buffer(write_data_.front().second),
            write_data_.front().first,
            [this](boost::system::error_code, std::size_t){
				INFO("Send a message to %s", write_data_.front().first.address().to_string().c_str());
                write_data_.pop_front();
                if(!write_data_.empty()){
                    do_write();
                }
            });
}

}

