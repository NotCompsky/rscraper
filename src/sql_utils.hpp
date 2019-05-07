#ifndef __MYSU__
#define __MYSU__
namespace mysu {

#include "rscraper_utils.hpp" // for SQL_*, init_mysql_from_file

void init(const char* fp){
    init_mysql_from_file(fp);
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
}
}
#endif
