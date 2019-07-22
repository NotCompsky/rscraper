#include "init_regexp_from_file.hpp"

#include <compsky/mysql/query.hpp>
#include <compsky/regex/named_groups.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace filter_comment_body {

boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;


void init_regexp_from_file(std::vector<char*>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents){
	compsky::mysql::query_buffer(&RES1,  "SELECT data FROM longstrings WHERE name='cmnt_body_regex'");
	char* regexpr_str;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &regexpr_str)){
		// Only one iteration
		compsky::regex::convert_named_groups(regexpr_str,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents);
		// Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst

		regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str,  boost::regex::perl | boost::regex::optimize);
	}
}

}
