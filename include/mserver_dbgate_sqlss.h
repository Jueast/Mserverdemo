#ifndef __MSERVER_DBGATE_SQLSS__
#define __MSERVER_DBGATE_SQLSS__
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
sql_create_3(users_salts, 1, 3,
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_varchar, username,
        mysqlpp::sql_char, salt);

sql_create_2(world, 1, 2,
        mysqlpp::sql_timestamp, ts,
        mysqlpp::sql_long_varbinary, dat);
    
sql_create_3(player, 1, 3,
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_timestamp, ts,
        mysqlpp::sql_long_varbinary, dat); 
#endif
