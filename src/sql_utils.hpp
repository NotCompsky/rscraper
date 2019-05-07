#ifndef __MYSU__
#define __MYSU__
namespace mysu {

#include "rscraper_utils.hpp" // for SQL_*, init_mysql_from_file


constexpr const char* SELECT_SUB_NAME_PRE = "SELECT id FROM subreddit WHERE name = \"";
char SELECT_SUB_NAME[strlen(SELECT_SUB_NAME_PRE) + 128+1 + 1] = "SELECT id FROM subreddit WHERE name = \"";


void init(const char* fp){
    init_mysql_from_file(fp);
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
}

uint64_t get_subreddit_id(const char* name){
    auto i = strlen(SELECT_SUB_NAME_PRE);
    memcpy(SELECT_SUB_NAME + i,  name,  strlen(name));
    i += strlen(name);
    SELECT_SUB_NAME[i++] = '"';
    SELECT_SUB_NAME[i] = 0;
    
    SQL_RES = SQL_STMT->executeQuery(SELECT_SUB_NAME);
    
    if (SQL_RES->next())
        return SQL_RES->getUInt64(1);
    return 0;
}

}
#endif
