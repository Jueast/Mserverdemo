#include <ctime>
#include <array>
#include <deque>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include "packed_message.hpp"


using boost::asio::ip::tcp;
typedef std::deque<PackedMessage> PackedMessageQueue;
std::string make_daytime_string()
{
  using namespace std; // For time_t, time and ctime;
  time_t now = time(0);
  return ctime(&now);
}

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
    typedef boost::shared_ptr<tcp_connection> pointer;

    static pointer create(boost::asio::io_service& io_service){
        return pointer(new tcp_connection(io_service));
    }

    tcp::socket& socket(){
        return socket_;
    }

    void start()
    {
        
        PackedMessage pm;
        build_packed_message(pm, 0, Request::LOGIN, make_daytime_string());
        deliver(pm);
        reg_read();
    }

private:
    tcp_connection(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
    }
    void reg_read(){
        boost::asio::async_read(socket_, boost::asio::buffer(pm_.data(), PackedMessage::header_length),
            boost::bind(&tcp_connection::handle_read_header, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
        std::cout << "Read regiseted!" << std::endl;

    }
    void handle_read_header(const boost::system::error_code& ec,
              size_t){
        if(ec)
            throw boost::system::error_code(ec);
        //std::cout << "Handle Now!" << std::endl;
        size_t l = pm_.decode_header();
        //std::cout << "...packet size: " << l << std::endl;
        boost::asio::async_read(socket_, 
                boost::asio::buffer(pm_.body(), pm_.body_length()),
                boost::bind(&tcp_connection::handle_read_body, shared_from_this(),
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
        //std::cout << "...read body regiseted!" << std::endl;
  }
  void handle_read_body(const boost::system::error_code& ec,
          size_t){
        if(ec)
            throw boost::system::error_code(ec);
        Request r = pm_.decode_request();
        std::cout << "-----------------Request Info------------------------" << std::endl;
        std::cout << "ID: " << r.id() << std::endl;
        std::cout << "CONTENT: " << r.content() << std::endl;
        std::cout << "-----------------------------------------------------" << std::endl;
        PackedMessage pm;
        build_packed_message(pm, 0, Request::LOGIN, make_daytime_string());
        deliver(pm);
        reg_read();
  }
  void deliver(const PackedMessage& pm){
      bool write_in_progress = !pm_queue_.empty();
      pm_queue_.push_back(pm);
      if(!write_in_progress){
          do_write();
      }

  }
  void do_write(){
        std::cout << pm_queue_.front().decode_header() << std::endl; 
        boost::asio::async_write(socket_, boost::asio::buffer(pm_queue_.front().data(), pm_queue_.front().whole_size()),
                        boost::bind(&tcp_connection::handle_write, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

  }
  void handle_write(const boost::system::error_code& ec,
      size_t)
  {
      if(ec)
            throw boost::system::error_code(ec);
      std::cout << "WRITE COMPLETE" << std::endl;
      if(!ec){
          pm_queue_.pop_front();
          if(!pm_queue_.empty()){
            do_write(); 
          }
      }
      else{
          std::cout << "SOCKET DOWN!" << std::endl;
          socket_.close();
      }
  }
  tcp::socket socket_;
  PackedMessage pm_;
  PackedMessageQueue pm_queue_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), 8000))
  {
    start_accept();
  }

private:
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(acceptor_.get_io_service());
    cons.push_back(new_connection);
    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    std::cout << "ADD A CONNECTION" << std::endl;
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
  std::vector<tcp_connection::pointer> cons;
};

int main()
{
  try
  {
    boost::asio::io_service io_service;
    tcp_server server(io_service);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
