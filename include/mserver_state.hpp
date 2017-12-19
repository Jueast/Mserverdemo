#include "mpack.pb.h"
#include <deque>
#include <boost/asio.hpp>
namespace MState {

struct Attribute {
    std::string name;
    int32_t value;
};

typedef std::unordered_map<uint32_t, Attribute> Transaction;
typedef std::deque<MNet::Mpack> TaskQueue;
}


class StateManager {
public:
    static StateManager& getStateMgr();
    void init(const char* filename);   
	void addTask(MNet::Mpack m);
	void sync();
    void load();
	void modify();
	void dispatch();
private:
    StateManager();
    ~StateManager() = default;
	boost::asio::deadline_timer timer_;
    std::unordered_map<uint32_t, MState::Transaction> player_state_;
    MState::Transaction world_state_;
	MState::TaskQueue queue_;
    MNet::Mpack sync_message_;

};



