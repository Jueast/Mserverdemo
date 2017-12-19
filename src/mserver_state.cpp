#include "mserver_state.hpp"
#include "mserver_net.hpp"

StateManager& StateManager::getStateMgr() 
{
	static StateManager mgr;
	return mgr;
}

StateManager::StateManager()
:timer_(NetworkManager::getNetMgr().get_io_service()),
 player_state_(),
 world_state_(),
 queue_(),
 sync_message_()
{
}

void StateManager::sync() 
{
}
