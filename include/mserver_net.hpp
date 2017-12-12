#ifndef __MSERVER_NET__
#define __MSERVER_NET__
#include <deque>
#include <boost/asio.hpp>
#include "mpack_message.hpp"
namespace MNet
{

using boost::asio::ip::tcp;
typedef std::deque<MpackMessage> MpackMessageList;
class NetworkSession;
//inherited from enable... to make it easy to produce shared pointers.
class NetworkConnection : std::enable_shared_from_this<NetworkConnection> 
{
public:
    NetworkConnection(tcp::socket socket, NetworkSession& session) 
        : socket_(std::move(socket)),
          session_(session)
    {
    }
    void start();
    void deliver(const Mpack& mm );
private:
    void do_read_header();
    void do_read_body();
    void do_write();
    tcp::socket socket_;
    NetworkSession& session_;
    MpackMessage read_message_;
    MpackMessageList write_message_list_;
};
class TCPServer;
class NetworkSession : std::enable_shared_from_this<NetworkSession>{
public:
    enum SessionState {
        SESSION_UNAUTHORIZED = 0,
        SESSION_AUTHORIZED = 1,
        SESSION_QUERYING = 2,
        SESSION_MODIFYING = 3
    };
	NetworkSession(tcp::socket socket, TCPServer& server) 
			: conn_(std::move(socket), *this),
			  server_(server){}
    void start();
    void dispatch(Mpack m);
    void close();
private:
    NetworkConnection conn_;
    SessionState state_;
	TCPServer& server_;
};

class TCPServer {
public:
	TCPServer(boost::asio::io_service& io_service,
			  tcp::endpoint& ep):
			acceptor_(io_service, ep),
			socket_(io_service)					
	{
	}
private:
	tcp::acceptor acceptor_;
	tcp::socket socket_;
	std::unordered_map<int, std::shared_ptr<NetworkSession>> sessions_;

};
}
class NetworkManager{
public:
    static NetworkManager& getNetMgr();
    static void init(const char* filename);
	boost::asio::io_service& get_io_service()
	{
		return io_service_;
	}
private:
	boost::asio::io_service io_service_;
    NetworkManager() = default;
    ~NetworkManager() = default;
};



#endif
