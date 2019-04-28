/* MySQL */
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Driver* SQL_DRIVER = get_driver_instance();
sql::Connection* SQL_CON;
sql::Statement* SQL_STMT;
sql::ResultSet* SQL_RES;

extern "C"
void init_mysql_from_file(const char* fp){
    FILE* f = fopen(fp, "r");
    size_t size;
    char* mysql_url = nullptr;
    char* mysql_usr = nullptr;
    char* mysql_pwd = nullptr;
    getline(&mysql_url, &size, f);
    getline(&mysql_usr, &size, f);
    getline(&mysql_pwd, &size, f);
    mysql_url[strlen(mysql_url)-1] = 0; // Remove trailing newline
    mysql_usr[strlen(mysql_usr)-1] = 0; // Remove trailing newline
    mysql_pwd[strlen(mysql_pwd)-1] = 0; // Remove trailing newline
    SQL_CON = SQL_DRIVER->connect(mysql_url, mysql_usr, mysql_pwd);
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
}
