#include "init_regexp_from_file.hpp"

#include <compsky/mysql/query.hpp>
#include <compsky/regex/named_groups.hpp>


namespace _mysql {
	extern MYSQL* obj;
	extern MYSQL_RES* res1;
	extern MYSQL_ROW row1;
}


namespace filter_comment_body {

boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;


void init_regexp_from_file(std::vector<char*>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents){
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1,  "SELECT data FROM longstrings WHERE name='cmnt_body_regex'");
	char* regexpr_str;
	while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &regexpr_str)){
		// Only one iteration - the next call to assign_next_row frees the memory
		compsky::regex::convert_named_groups(regexpr_str,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents);

		regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str,  boost::regex::perl | boost::regex::optimize);
	}
}

}
