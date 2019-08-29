#ifndef RSCRAPER_HUB_MYSQL_DECLARATIONS
#define RSCRAPER_HUB_MYSQL_DECLARATIONS


#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/mysql.h>

extern char BUF[];

namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_RES* res1;
	extern MYSQL_RES* res2;
	extern MYSQL_ROW row1;
	extern MYSQL_ROW row2;
}

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::StrLen strlen;
}


#endif
