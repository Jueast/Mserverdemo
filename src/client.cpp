#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <array>
#include <deque>
#include <thread>
#include "packed_message.hpp"
#include "logging.hpp"
using boost::asio::ip::tcp;
typedef std::deque<PackedMessage> PackedMessageQueue;



class Client {
public:
    Client(boost::asio::io_service& io_service,
            tcp::resolver::iterator endpoint_iterator)
        : io_service_(io_service),
          socket_(io_service)
    {
        do_connect(endpoint_iterator);      
    }
    
    void write(const PackedMessage& pm){
        io_service_.post(
                [this, pm](){
                    bool write_in_progress = !write_msg_queue_.empty();
                    write_msg_queue_.push_back(pm);
                    if(!write_in_progress){
                        do_write();
                    }
                });
    }

    void close(){
        io_service_.post([this]() { socket_.close();});
    }
private:
    void do_connect(tcp::resolver::iterator endpoint_iterator){
        boost::asio::async_connect(socket_, endpoint_iterator,
                [this](boost::system::error_code ec, tcp::resolver::iterator)
                {
                    if(!ec){
                        do_read_header();
                    }
                });
    }

    void do_read_header(){
        boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.data(), PackedMessage::header_length),
                [this](boost::system::error_code ec, std::size_t)
                {
                    read_msg_.decode_header();
                    if(!ec)
                    {
                            do_read_body();
                    }
                    else
                    {
                        std::cout << "SOCKET DOWN" << std::endl;
                        socket_.close();
                    }
                });
    }
    
    void do_read_body(){
        boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
                [this](boost::system::error_code ec, std::size_t)
                {
                    if(!ec){
                        Request r = read_msg_.decode_request();
                        output(r, std::cout);
                        do_read_header();
                    }
                    else{
                        std::cout << "SOCKET DOWN in do_read_body" << std::endl;
                        throw boost::system::system_error(ec);
                        socket_.close();
                    }
                });
    }
    void do_write(){
        boost::asio::async_write(socket_,
                boost::asio::buffer(write_msg_queue_.front().data(),
                    write_msg_queue_.front().whole_size()),
                [this](boost::system::error_code ec, std::size_t)
                {
                    if(!ec)
                    {
                        write_msg_queue_.pop_front();
                        if(!write_msg_queue_.empty()){
                            do_write();
                        }
                    }
                    else{
                        socket_.close();
                    }
                });
    }
    void output(const Request& r, std::ostream& s){
        s << "----------------MESSAGE INFO-------------------" << std::endl;
        s << "ID :  " << r.id() << std::endl;
        s << "CONTENT: " << r.content() << std::endl;
        s << "-----------------------------------------------" << std::endl;
    }
    boost::asio::io_service& io_service_;
    tcp::socket socket_;
    PackedMessage read_msg_;
    PackedMessageQueue write_msg_queue_;
};
int id = 1;


int main(int argc, const char* argv[]){
    try{
        if(argc != 3){
            std::cerr << "Usage: client <host> <port>" << std::endl;
            return 1;
        }
        using logging::Logger;
        using logging::level::level_enum;
        Logger::getLogger().setFileName("log/test.log");
        Logger::getLogger().setLogLevel(level_enum::fatal);
        INFO("Hello, %s", "world");
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(argv[1], argv[2]);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        Client c(io_service, endpoint_iterator);
        std::thread t([&io_service](){ io_service.run();});
        std::string s;
        while(std::getline(std::cin, s)){
            PackedMessage pm;
            build_packed_message(pm, id, Request::LOGIN, s);
            c.write(pm);
        
        }
        c.close();
        t.join();
    }
    catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
