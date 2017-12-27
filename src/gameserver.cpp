#include <boost/asio.hpp>
#include "mserver_net.hpp"
#include "mserver_state.hpp"
#include "logging.hpp"
#include "common.hpp"
int main(int argc, char* argv[]){
    try{
	NetworkManager& netMgr = NetworkManager::getNetMgr();
    StateManager& stateMgr = StateManager::getStateMgr(); 
	Logging::Logger& logger = Logging::Logger::getLogger();
	logger.setFileName("log/gameserver.log");
	logger.setLogLevel(Logging::level::fatal);
	if(argc > 2)
	{
	    std::cerr << "Usage: gameserver [configure filename ('gameserver.conf' as default)]\n";
	    exit(1);
	}
	if(argc == 2)
	{
	    netMgr.init(argv[1]);
	}
	else
	{
	    netMgr.init("gameserver.conf");
	}
	get_io_service().run();
    }
    catch(std::exception& e)
    {
	std::cerr << e.what() << std::endl;
    }
    return 0;
}


