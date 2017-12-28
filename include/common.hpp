#ifndef __COMMON__
#define __COMMON__
#include <boost/asio.hpp>
#include "mpack.pb.h"
inline boost::asio::io_service& get_io_service()
{
    static boost::asio::io_service i;
    return i;
}
// update means the data in left will be changed according to right;
// CONTENTS determined by right;
void update_player(MNet::Player& pl, const MNet::Player& pr);
void update_world(MNet::World& wl, const MNet::World& wr);

// load means thed blank in left will be filled according to right;
// CONTENTS determined by left;
void load_player(MNet::Player& pl, const MNet::Player& pr);
void load_world(MNet::World& wl, const MNet::World& wr);
#endif
