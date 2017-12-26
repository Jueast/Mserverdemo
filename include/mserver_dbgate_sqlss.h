#ifndef __MSERVER_DBGATE_SQLSS__
#define __MSERVER_DBGATE_SQLSS__
#include <mysql++/mysql++.h>
#include <mysql++/ssqls.h>
sql_create_3(users_salts, 1, 3,
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_varchar, username,
        mysqlpp::sql_char, salt);

sql_create_11(player_characteristic, 1, 11,
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_int_unsigned, STR,
        mysqlpp::sql_int_unsigned, CON,
        mysqlpp::sql_int_unsigned, SIZ,
        mysqlpp::sql_int_unsigned, DEX,
        mysqlpp::sql_int_unsigned, APP,
        mysqlpp::sql_int_unsigned, INT,
        mysqlpp::sql_int_unsigned, POW,
        mysqlpp::sql_int_unsigned, EDU,
        mysqlpp::sql_int_unsigned, LUC,
        mysqlpp::sql_int_unsigned, AGE);

sql_create_8(player_skill, 1, 8, 
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_int_unsigned, COMBAT,
        mysqlpp::sql_int_unsigned, KNOWLEDGE,
        mysqlpp::sql_int_unsigned, SENSE,
        mysqlpp::sql_int_unsigned, COMMUNICATION,
        mysqlpp::sql_int_unsigned, OPERATION,
        mysqlpp::sql_int_unsigned, HIDE,
        mysqlpp::sql_int_unsigned, OBSERVE);

sql_create_5(player_state, 1, 5,
        mysqlpp::sql_int_unsigned, uid,
        mysqlpp::sql_int_unsigned, HP,
        mysqlpp::sql_int_unsigned, SAN,
        mysqlpp::sql_int_unsigned, MONEY,
        mysqlpp::sql_int_unsigned, MAGIC);

sql_create_2(world, 1, 2,
        mysqlpp::sql_timestamp, ts,
        mysqlpp::sql_int_unsigned, num_of_players);
    

#endif
