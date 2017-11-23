#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <array>
#include "packed_message.hpp"

using boost::asio::ip::tcp;

int main(int argc, const char* argv[]){
    try{
        if(argc != 3){
            std::cerr << "Usage: client <host> <message>" << std::endl;
            return 1;
        }
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(argv[1], "8000");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::socket socket(io_service);
        boost::asio::connect(socket, endpoint_iterator);

        Request r;
        PackedMessage pm;
        r.set_type(Request::LOGIN);
        r.set_id(0);
        r.set_content(std::string(argv[2]));
        pm.encode_request(r);

        boost::system::error_code ignored_error;
        boost::asio::write(socket, boost::asio::buffer(pm.data(), pm.whole_size()), ignored_error);
        std::cout << "Send OK!" << std::endl;
        for(;;){
            std::array<char, 128> buf;
            boost::system::error_code error;
            size_t len = socket.read_some(boost::asio::buffer(buf), error);
            if(error == boost::asio::error::eof)
                break;
            else if(error)
                throw boost::system::system_error(error);
            std::cout.write(buf.data(), len);
        }
    }
    catch (std::exception& e){
        std::cerr << e.what() << std::endl;
    }
}
