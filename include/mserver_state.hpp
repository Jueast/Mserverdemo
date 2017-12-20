#ifndef __MSERVER_STATE__
#define __MSERVER_STATE__
#include <deque>
#include <boost/asio.hpp>
#include "mpack.pb.h"
#include "common.hpp"
namespace MState {

using MNet::States_Attribute;
typedef std::unordered_map<uint32_t, States_Attribute> Transaction;
typedef std::deque<MNet::Mpack> TaskQueue;


}


class StateManager {
public:
    static StateManager& getStateMgr();
    void init(const char* filename);   
    void addTask(MNet::Mpack m);
private:
    StateManager();
    ~StateManager() = default;
    void do_sync();
    void do_sync_complete(MNet::Mpack m);
    void do_load(MNet::Mpack m);
    void do_modify(MNet::Mpack m);
    void do_process();
    boost::asio::deadline_timer timer_;
    boost::asio::strand strand_;
    std::unordered_map<uint32_t, MState::Transaction> player_state_;
    MState::Transaction world_state_;
    MState::TaskQueue queue_;
    MNet::Mpack sync_message_;
    MNet::Mpack send_message_;
};


#endif
