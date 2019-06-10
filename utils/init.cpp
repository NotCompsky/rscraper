#include <stdlib.h> // for getenv
#include <stdio.h> // for fprintf

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <mysql/mysql.h>


MYSQL_RES* RES1;
MYSQL_ROW ROW1;

namespace compsky::asciify {
    char* BUF = (char*)malloc(1024);
}


int diy(const char* a,  const char* b){
    fprintf(stderr, "%s%s\nFor other setups, you will have to set up the database manually\nCreate the database, create user and grant permissions if necessary, and copy-paste the commands from 'init.sql' into your MySQL client\n",  a,  b);
    return 1;
}


int main(const int argc,  char** argv){
    const char* cfg_pth = argv[1]; // Cannot use getenv("RSCRAPER_MYSQL_CFG") because we expect to run this as root, a different environment
    if (cfg_pth == nullptr){
        fprintf(stderr, "Environmental variable 'RSCRAPER_MYSQL_CFG' not set\n");
        exit(1);
    }
    compsky::mysql::init_auth(cfg_pth);
    
    const char* hostname = compsky::mysql::MYSQL_AUTH[0];
    
    if (strncmp(hostname, "localhost", strlen("localhost")) != 0)
        return diy("rscraper-init assumes the MySQL server is local, not ",  hostname);
    
    const char* username = compsky::mysql::MYSQL_AUTH[2]; // User to grant permissions to
    const char* password = compsky::mysql::MYSQL_AUTH[3]; // His password
    if (argc == 2){
        compsky::mysql::MYSQL_AUTH[2] = "root";
        compsky::mysql::MYSQL_AUTH[3] = nullptr;
    } else {
        compsky::mysql::MYSQL_AUTH[2] = argv[1];  // username
        compsky::mysql::MYSQL_AUTH[3] = argv[2]; // password
    }
    const char* db_name = compsky::mysql::MYSQL_AUTH[4];
    compsky::mysql::MYSQL_AUTH[4] = nullptr; // Haven't created the database yet
    compsky::mysql::login_from_auth();
    
    compsky::mysql::exec("CREATE DATABASE IF NOT EXISTS `", db_name, "`");
    
    mysql_select_db(&compsky::mysql::OBJ, db_name); // Set as current database
    
    constexpr static const char* stmts = 
        #include "init.sql" // kayahr's idea from here https://stackoverflow.com/questions/410980/include-a-text-file-in-a-c-program-as-a-char
    ;
    
    /* The following is an alternative to using `mysql_set_server_option(&compsky::mysql::OBJ, MYSQL_OPTION_MULTI_STATEMENTS_ON);`
     * The reason it is used is that it is easier to debug the SQL commands individually
     */
    char* stmt = const_cast<char*>(stmts);
    for (char* itr = stmt;  *itr != 0;  ++itr){
        if (unlikely(*itr == ';')){
            compsky::mysql::exec_buffer(stmt,  (uintptr_t)itr - (uintptr_t)stmt);
            stmt = itr + 1;
        }
    }
    
    constexpr static const compsky::asciify::flag::Escape esc;
    
    compsky::mysql::exec("CREATE USER IF NOT EXISTS `", esc, '`', username, "`@`localhost` IDENTIFIED BY \"", esc, '"', password, "\"");
    
    compsky::mysql::exec("GRANT SELECT, INSERT, UPDATE ON ", db_name, ".* TO `", esc, '`', username, "`@`localhost`");
    
    compsky::mysql::exit();
    
    return 0;
    
    
    goto__diy:
    
    
    return 1;
}
