#ifndef __NETWORK_MANAGER__
#define __NETWORK_MANAGER__
#include <boost/asio.hpp>

using boost::asio::ip::tcp;
class NetworkSession;
class NetworkConnection {
public:
    NetworkConnection(tcp::socket socket, NetworkSession& session) 
        : socket_(std::move(socket)),
          session_(session)
    {
    }
private:
    tcp::socket socket_;
    NetworkSession& session_;
};

class NetworkSession : std::enable_shared_from_this<NetworkSession>{

private:
    tcp::socket socket_;


};
class NetworkManager{
public:
    static NetworkManager& getNetMgr();
private:
    NetworkManager() = default;
    ~NetworkManager() = default;
};



#endif
