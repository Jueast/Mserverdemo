#include "logging.hpp"
#include "mserver_net_connection.hpp"
#include "mserver_net.hpp"
namespace MNet 
{

void NetworkConnection::start()
{
    do_read_header();
}

void NetworkConnection::deliver(const Mpack& m)
{	
    bool write_in_progress = !write_message_list_.empty();
    write_message_list_.emplace_back(m);
    if(!write_in_progress)
    {
        do_write();
    }

}
void NetworkConnection::do_read_header()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_message_.data(), MpackMessage::header_length),
        [this, self](boost::system::error_code ec, std::size_t l)
        {
            if(!ec && read_message_.decode_header())
            {
               do_read_body(); 
            }
            else
            {
                if(ec)
                {
                    ERROR("boost asio error: %s", ec.message().c_str());  
                // too long mpack means this connection should be abandoned.
                }
                else
                {
                    ERROR("Too long Mpack from %s", socket_.remote_endpoint().address().to_string().c_str());
                }
                session_.close();             
            }
        });
}

void NetworkConnection::do_read_body()
{
    auto self(shared_from_this());
    boost::asio::async_read(socket_,
            boost::asio::buffer(read_message_.body(), read_message_.body_length()),
            [this, self](boost::system::error_code ec, std::size_t l)
            {
                if(!ec)
                {
                    Mpack m;
		    m.ParsePartialFromArray(read_message_.body(), l);
		    session_.dispatch(std::move(m));
                    do_read_header();
                }
                else
                {
                    ERROR("boost asio error: %s", ec.message().c_str());
                    session_.close();
                }
            });
    
}

void NetworkConnection::do_write()
{
    auto self(shared_from_this());
    boost::asio::async_write(socket_,
            boost::asio::buffer(write_message_list_.front().data(), 
                                write_message_list_.front().whole_size()),
            [this, self](boost::system::error_code ec, std::size_t l)
            {
                if(!ec)
                {
                    write_message_list_.pop_front();
                    if(!write_message_list_.empty())
                    {
                        do_write();
                    }
                }
                else
                {
                    ERROR("boost asio error: %s", ec.message().c_str());
                }

            });

}

}
