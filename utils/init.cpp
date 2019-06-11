#include <stdlib.h> // for getenv
#include <stdio.h> // for fprintf
#include <iostream> // for std::cout

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <mysql/mysql.h>


MYSQL_RES* RES1;
MYSQL_ROW ROW1;

namespace compsky::asciify {
    char* BUF = (char*)malloc(1024);
}


constexpr static const char* stmts =
    #include "init.sql"
;

char* auth_ptr;

void ef_reed(){
    char c;
    while((c=fgetc(stdin))){
        if (c != '\n'){
            *auth_ptr = c;
            ++auth_ptr;
        } else return;
    }
}


int main(const char* stmts){
    size_t len;
    FILE* cfg;
    
    std::cout << "* MySQL Configuration *" << std::endl;
    
    std::cout << "Absolute path to save the config file to: ";
    char* cfg_pth;
    getline(&cfg_pth, &len, stdin);
    
    char auth[4096];
    memcpy(auth, "HOST: ", 6);
    char* auth_ptr_ends[6];
    auth_ptr = auth + 6;
    compsky::mysql::MYSQL_AUTH[0] = auth_ptr;
    auto i = 0;
    
    std::cout << "Host (localhost if it is on this machine): ";
    ef_reed();
    auth_ptr_ends[i] = auth_ptr;
    memcpy(auth_ptr, "\nPATH: ", 7);
    auth_ptr += 7;
    compsky::mysql::MYSQL_AUTH[++i] = auth_ptr;
    
    bool is_localhost = (strncmp(compsky::mysql::MYSQL_AUTH[i-1], "localhost", strlen("localhost")) == 0);
    if (is_localhost){
        FILE* proc = popen("mysql_config --socket", "r");
        auth_ptr += fread(auth_ptr, 1, 1024, proc) - 1; // Overwrite trailing newline
        pclose(proc);
    } else {
        std::cout << "Path: ";
        ef_reed();
    }
    auth_ptr_ends[i] = auth_ptr;
    memcpy(auth_ptr, "\nUSER: ", 7);
    auth_ptr += 7;
    compsky::mysql::MYSQL_AUTH[++i] = auth_ptr;
    
    std::cout << "Username: ";
    ef_reed();
    auth_ptr_ends[i] = auth_ptr;
    memcpy(auth_ptr, "\nPWRD: ", 7);
    auth_ptr += 7;
    compsky::mysql::MYSQL_AUTH[++i] = auth_ptr;
    
    std::cout << "Corresponding password: ";
    ef_reed();
    auth_ptr_ends[i] = auth_ptr;
    memcpy(auth_ptr, "\nDBNM: ", 7);
    auth_ptr += 7;
    compsky::mysql::MYSQL_AUTH[++i] = auth_ptr;
    
    std::cout << "Database name: ";
    ef_reed();
    auth_ptr_ends[i] = auth_ptr;
    memcpy(auth_ptr, "\nPORT: ", 7);
    auth_ptr += 7;
    compsky::mysql::MYSQL_AUTH[++i] = auth_ptr;
    
    if (is_localhost){
        *auth_ptr = '0';
        ++auth_ptr;
    } else {
        std::cout << "Port number: ";
        ef_reed();
    }
    auth_ptr_ends[i] = auth_ptr;
    *auth_ptr = '\n'; // Important that there is a trailing newline
    
    cfg = fopen(cfg_pth, "wb");
    fwrite(auth,  1,  (uintptr_t)auth_ptr + 1 - (uintptr_t)auth,  cfg);
    fclose(cfg);
    
    for (auto j = 0;  j < 6;  ++j)
        auth_ptr_ends[j][0] = 0;
    for (auto j = 0;  j < 6;  ++j)
        printf("MYSQL_AUTH[%d] = %s\n", j, compsky::mysql::MYSQL_AUTH[j]);
    
    /* Now to login to the MySQL database with the root user */
    
    const char* username = compsky::mysql::MYSQL_AUTH[2]; // User to grant permissions to
    const char* password = compsky::mysql::MYSQL_AUTH[3]; // His password
    
    std::cout << "MySQL admin username: ";
    ef_reed();
    compsky::mysql::MYSQL_AUTH[2] = auth_ptr;
    
    const uintptr_t auth_ptr_before = (uintptr_t)auth_ptr;
    *auth_ptr = 0; // So we can write it out here:
    std::cout << "MySQL admin password (leave blank to use system socket authentication - i.e. if you can login to MySQL as `" << compsky::mysql::MYSQL_AUTH[2] << "` without a password): ";
    ef_reed();
    if (auth_ptr_before == (uintptr_t)auth_ptr)
        compsky::mysql::MYSQL_AUTH[3] = nullptr;
    else
        compsky::mysql::MYSQL_AUTH[3] = auth_ptr;
    
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
    
    if (is_localhost){
        compsky::mysql::exec("CREATE USER IF NOT EXISTS `", esc, '`', username, "`@`localhost` IDENTIFIED BY \"", esc, '"', password, "\"");
        compsky::mysql::exec("GRANT SELECT, INSERT, UPDATE ON ", db_name, ".* TO `", esc, '`', username, "`@`localhost`");
    } else {
        std::cout << "You must manually create the user `" << username << "` and grant him permissions: SELECT, INSERT, UPDATE on `" << db_name << "`" << std::endl;
    }
    
    compsky::mysql::exit();
    
    return 0;
}
