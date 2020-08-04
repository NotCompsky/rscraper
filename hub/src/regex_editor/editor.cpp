/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifdef USE_BOOST_REGEX

#include "editor.hpp"
#include <compsky/mysql/query.hpp>


namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_RES* res1;
	extern MYSQL_ROW row1;
}

namespace _f {
	using namespace compsky::asciify::flag;
	constexpr static Escape esc;
}


void RScraperRegexEditor::load_file(){
	compsky::mysql::query(_mysql::obj, _mysql::res1,  this->buf, "SELECT data FROM longstrings WHERE name='", this->src, "'");
	const char* data_;
	while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &data_))
		this->set_text(data_);
}

void RScraperRegexEditor::save_to_file(){
	QString buf;
	const int buf_sz = this->get_text().size() + 1; // Extra char for trailing \0
	buf.reserve(buf_sz);
	if (!this->to_final_format(this->does_user_want_optimisations(), buf, 0, 0))
		return;
	
	this->ensure_buf_sized(buf_sz);
	
	compsky::mysql::exec(_mysql::obj, this->buf, "UPDATE longstrings SET data=\"", _f::esc, '"', this->get_text(), "\" WHERE name='", this->src, "'");
	compsky::mysql::exec(_mysql::obj, this->buf, "UPDATE longstrings SET data=\"", _f::esc, '"', buf, "\" WHERE name='", this->dst, "'");
}

#endif
