#include <compsky/mysql/create_config.hpp>


int main(){
    compsky::mysql::create_config(
        #include "init.sql"
        , "RSCRAPER_MYSQL_CFG"
    );
    
    return 0;
}
