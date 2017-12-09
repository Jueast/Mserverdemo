#include <mysql++/mysql++.h>
namespace MDB 
{

typedef std::shared_ptr<mysqlpp::Connection> MysqlConnPtr;

class MDBManager {
public:
    void init();
    MysqlConnPtr grab();
    static MDBManager& getMDBMgr();
private:
    MDBManager() = default;
    ~MDBManager() = default;
    std::vector<MysqlConnPtr> conns_;
};

}


