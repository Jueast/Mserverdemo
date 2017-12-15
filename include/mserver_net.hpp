#ifndef __MSERVER_NET__
#define __MSERVER_NET__
#include <deque>
#include <boost/asio.hpp>
#include "mpack_message.hpp"
#include "mserver_net_connection.hpp"
namespace MNet
{
class TCPServer;
class NetworkSession : public std::enable_shared_from_this<NetworkSession>{
public:
    enum SessionState {
        SESSION_UNAUTHORIZED = 0,
        SESSION_AUTHORIZED = 1,
        SESSION_QUERYING = 2,
        SESSION_MODIFYING = 3
    };
    NetworkSession(tcp::socket socket, TCPServer& server, uint32_t session_id) 
		    : conn_(std::move(socket), *this),
                      state_(SESSION_UNAUTHORIZED),
		      server_(server),
                      session_id_(session_id){}
    void start();
    void dispatch(Mpack m);
    void deliver(Mpack r);
    void close();
private:
    void dispatch_unauthorized(Mpack m);
    void deliver_unauthorized(Mpack r);
    NetworkConnection conn_;
    SessionState state_;
    TCPServer& server_;
    uint32_t session_id_;
};

class TCPServer {
public:
	TCPServer(boost::asio::io_service& io_service,
		  tcp::endpoint& ep,
                  uint32_t max_num_of_sessions);
        typedef std::shared_ptr<NetworkSession> NetSessionPtr;
        NetSessionPtr create_session();
        void release_session(NetworkSession * p, uint32_t x);
        void deliver(Mpack r);
        void close(uint32_t session_id_);
private:
        void do_accept();
	tcp::acceptor acceptor_;
	tcp::socket socket_;
        std::deque<uint32_t> session_nums_;
	std::unordered_map<uint32_t, NetSessionPtr> sessions_;

};
using boost::asio::ip::udp;
class UDPServer {
public:
    UDPServer(boost::asio::io_service& io_service,
	      udp::endpoint self_address,
              udp::endpoint dbgate_address)
	    : recv_socket_(io_service, self_address),
              dbgate_address_(dbgate_address){}
    udp::endpoint get_dbgate_address()
    {
        return dbgate_address_;
    }
//TODO implement UDPServer
private:
    udp::socket recv_socket_;
    udp::endpoint dbgate_address_;
};


}
using boost::asio::ip::udp;
class NetworkManager{
public:
    static NetworkManager& getNetMgr();
    void init(const char* filename);
    boost::asio::io_service& get_io_service()
    {
	return io_service_;
    }
    // blocking login.
    void login(MNet::Mpack m);
    


private:
    NetworkManager() = default;
    ~NetworkManager() = default;
    boost::asio::io_service io_service_;
    std::shared_ptr<MNet::TCPServer> tcp_server_ptr_;
    std::shared_ptr<MNet::UDPServer> udp_server_ptr_;
};



#endif
