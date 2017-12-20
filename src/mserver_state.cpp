#include "common.hpp"
#include "mserver_state.hpp"
#include "mserver_net.hpp"

StateManager& StateManager::getStateMgr() 
{
	static StateManager mgr;
	return mgr;
}

StateManager::StateManager()
:timer_(get_io_service()),
 strand_(get_io_service()),
 player_state_(),
 world_state_(),
 queue_(),
 sync_message_()
{
}

void StateManager::do_sync() 
{
    NetworkManager::getNetMgr().sync(send_message_);
}
void StateManager::do_sync_complete(MNet::Mpack m)
{
    if(!m.error()){
        send_message_ = sync_message_;
        sync_message_.Clear();
    }
}


void StateManager::addTask(MNet::Mpack m)
{
    strand_.post(
        [this, m](){
            bool processing = !queue_.empty();
            queue_.push_back(std::move(m));
            if(!processing)
                do_process();
        });
}

void StateManager::do_process() 
{
    using MNet::Mpack;
    strand_.post(
        [this](){
            Mpack m = queue_.front();
            switch(m.type()) 
            {
                case Mpack::STATE_QUERY:
                {
                    strand_.get_io_service().post(
                        [this, m](){do_load(m);});
                    break;
                }
                case Mpack::STATE_MODIFY:
                {
                    strand_.post(
                        [this, m](){do_modify(m);});
                    break;
                }
                case Mpack::CONTROL:
                {
                    strand_.post(
                        [this, m](){do_sync_complete(m);});
                    break;
                }
                default:
                    break;
            }
            queue_.pop_front();
            if(!queue_.empty())
                do_process();   
        });

}

void StateManager::do_load(MNet::Mpack m)
{
    using MNet::Mpack;
    using MNet::States;
    assert(m.has_states()); 
    MNet::States* states = m.mutable_states();
    
    for(auto p = states->mutable_player_attrs()->begin(); p != states->mutable_player_attrs()->end();p++) 
    {
        auto player = player_state_.at(p->first);
        for(auto a = p->second.mutable_attrs()->begin(); a != p->second.mutable_attrs()->end(); a++)
        {
            a->second = player.at(a->first);
        }
    }

    for(auto a = states->mutable_world_attrs()->begin(); a != states->mutable_world_attrs()->end();a++)
    {
        a->second = world_state_.at(a->first);
    }
   
    NetworkManager::getNetMgr().deliver(std::move(m));
}

void StateManager::do_modify(MNet::Mpack m)
{
    using MNet::Mpack;
    using MNet::States;
    assert(m.has_states()); 
    // update stage and sync_message simultaneously
    const MNet::States& states = m.states();
    MNet::States* sync_states = sync_message_.mutable_states(); 
    for(auto p = states.player_attrs().begin(); p != states.player_attrs().end();p++) 
    {
        auto player = player_state_.at(p->first);
        bool player_in_sync = sync_states->player_attrs().find(p->first) != sync_states->player_attrs().end();
        if(!player_in_sync){
            (*sync_states->mutable_player_attrs())[p->first] = p->second;
        }
        auto sync_player = (*sync_states->mutable_player_attrs())[p->first];   
        for(auto a = p->second.attrs().begin(); a != p->second.attrs().end(); a++)
        {
            player[a->first] = a->second;
            if(player_in_sync){
                (*sync_player.mutable_attrs())[a->first] = a->second;
            }
        }
    }

    for(auto a = states.world_attrs().begin(); a != states.world_attrs().end();a++)
    {
        world_state_[a->first] = a->second;
        (*sync_states->mutable_world_attrs())[a->first] = a->second;
    }
    Mpack r;
    r.set_type(Mpack::INFO);
    r.set_session_id(m.session_id());
    NetworkManager::getNetMgr().deliver(std::move(r));
}
