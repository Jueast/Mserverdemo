#ifndef __MSERVER_NET_CONNECTION__
#define __MSERVER_NET_CONNECTION__
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

}
#endif
