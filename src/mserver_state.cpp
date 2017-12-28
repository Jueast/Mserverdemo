#include "common.hpp"
#include "mserver_state.hpp"
#include "mserver_net.hpp"
#include "logging.hpp"


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
// TODO: Complete the init
void StateManager::init(const char* filename)
{
    
    timer_.expires_from_now(boost::posix_time::seconds(30));
    timer_.async_wait(strand_.wrap(std::bind(&StateManager::sync, this)));
    INFO("StateManager was initialized now.");
}
void StateManager::sync()
{
    do_sync();
    timer_.expires_at(timer_.expires_at() + boost::posix_time::seconds(30));
    timer_.async_wait(strand_.wrap(std::bind(&StateManager::sync, this)));
}
void StateManager::do_sync() 
{
    INFO("SYNC states now..");
    send_message_.set_type(MNet::Mpack::CONTROL);
    send_message_.set_control(MNet::Mpack::SYNC);
    NetworkManager::getNetMgr().sync(send_message_);
}
void StateManager::do_sync_complete()
{
    INFO("SYNC complete. flip.");
    send_message_ = sync_message_;
    sync_message_.Clear();
}


void StateManager::addTask(MNet::Mpack m)
{
    strand_.post(
        [this, m](){
            TRACE("Get task from session %u, waitting list size: %d", m.session_id(), queue_.size()); 
            bool processing = !queue_.empty();
            queue_.push_back(std::move(m));
            if(!processing)
                do_process();
        });
}

void StateManager::mountWorld(MNet::World w)
{
    world_state_ = std::move(w);
}

void StateManager::mountPlayer(uint32_t uid, MNet::Player p)
{
    INFO("Mount user %u now...", uid);
    if(player_state_.find(uid) == player_state_.end())
        player_state_[uid] = std::move(p);
    else{
        ERROR("This player %u is mounted!", uid);
    }
}

void StateManager::unmountPlayer(uint32_t uid)
{
    strand_.post(
        [this, uid](){
            INFO("Unmount player %u now.."); 
            if(player_state_.find(uid) == player_state_.end())
            {
                ERROR("This user %u is not mounted", uid);
            }
            else
                player_state_.erase(uid);
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
                    switch(m.control())
                    {
                        case Mpack::ACK_NO:
                        {
                        //    do_sync(); 
                            break;
                        }
                        case Mpack::ACK_YES:
                        {
                            strand_.post(
                                [this, m](){do_sync_complete();});
                            break;
                        };
                        default:break;
                    }
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
    INFO("Process query request from session %u...", m.session_id());
    for(auto it = m.mutable_players()->begin(); it != m.mutable_players()->end(); it++)
    {
        if(player_state_.find(it->first) == player_state_.end()){
            ERROR("Player %u is not mounted", it->first);
            continue;
        }
        loadPlayer(it->second, player_state_[it->first]);
    }
    if(m.has_world()){
        loadWorld(*m.mutable_world(), world_state_);
    }
    NetworkManager::getNetMgr().deliver(std::move(m));
}

void StateManager::do_modify(MNet::Mpack m)
{
    INFO("Process modify request from session %u...", m.session_id());
    for(auto it = m.players().begin(); it != m.players().end(); it++)
    {
        if(player_state_.find(it->first) == player_state_.end()){
            ERROR("Player %u is not mounted", it->first);
            continue;
        }
        updatePlayer(player_state_[it->first], it->second);
        if(sync_message_.players().find(it->first) == sync_message_.players().end())
        {
            (*sync_message_.mutable_players())[it->first] = it->second;
        }
        else
        {
            updatePlayer((*sync_message_.mutable_players())[it->first], it->second);
        }
    }
    if(m.has_world()){
        updateWorld(world_state_, m.world());
        if(sync_message_.has_world())
        {
            updateWorld(*sync_message_.mutable_world(), m.world());
        }
        else
        {
            (*sync_message_.mutable_world()) = m.world(); 
        }
    }

    MNet::Mpack r;
    r.set_type(MNet::Mpack::INFO);
    r.set_session_id(m.session_id());
    NetworkManager::getNetMgr().deliver(std::move(r));
}
