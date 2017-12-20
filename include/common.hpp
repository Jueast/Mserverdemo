#ifndef __COMMON__
#define __COMMON__
#include <boost/asio.hpp>
inline boost::asio::io_service& get_io_service()
{
    static boost::asio::io_service i;
    return i;
}
#endif
