/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include <compsky/mysql/create_config.hpp>
#include <compsky/asciify/init.hpp>

#include <string.h> // for strlen


namespace compsky {
	namespace asciify {
		char* BUF;
		char* ITR;
	}
}


int main(){
	constexpr static const char* sql =
		#include "init.sql"
	;

	if(compsky::asciify::alloc(strlen(sql) + 1024))
		return 1;

	compsky::mysql::create_config(
		sql
		, "SELECT, INSERT, UPDATE, DELETE"
		, "RSCRAPER_MYSQL_CFG"
	);
	
	return 0;
}
