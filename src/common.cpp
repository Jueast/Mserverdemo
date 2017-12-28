#include "common.hpp"

void update_player(MNet::Player& pl, const MNet::Player& pr)
{
    for(auto it = pr.attrs().begin(); it != pr.attrs().end(); it++)
    {
        (*pl.mutable_attrs())[it->first] = it->second;
    }
}

void update_world(MNet::World& wl, const MNet::World& wr)
{
    for(auto it = wr.attrs().begin(); it != wr.attrs().end(); it++)
    {
        (*wl.mutable_attrs())[it->first] = it->second;
    }
}

void load_player(MNet::Player& pl, const MNet::Player& pr)
{
    for(auto it = pl.mutable_attrs()->begin(); it != pl.mutable_attrs()->end(); it++)
    {
       it->second = pr.attrs().at(it->first);
    }
}

void load_world(MNet::World& wl, const MNet::World& wr)
{
    for(auto it = wl.mutable_attrs()->begin(); it != wl.mutable_attrs()->end(); it++)
    {
       it->second = wr.attrs().at(it->first);
    }
}
