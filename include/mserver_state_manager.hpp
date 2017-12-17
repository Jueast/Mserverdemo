#include "mpack.pb.h"
#include <boost/asio.hpp>
namespace MState {

struct Attribute {
    std::string name;
    int32_t value;
};

typedef std::unordered_map<uint32_t, Attribute> Transaction;


    

}




class StateManager {
public:
    static StateManager& getStateMgr();
    void init(const char* filename);   
    
private:
    StateManager() = default;
    ~StateManager() = default;
    std::unordered_map<uint32_t, MState::Transaction> PlayerState;
    MState::Transaction WorldState;
    MNet::Mpack SyncMessage;
};



