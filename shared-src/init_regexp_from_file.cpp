#include "init_regexp_from_file.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace filter_comment_body {

boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;


void init_regexp_from_file(std::vector<compsky::regex::CharPaar>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents){
	compsky::mysql::query_buffer(&RES1,  "SELECT data FROM longstrings WHERE name='cmnt_body_regex'");
	char* regexpr_str;
	if (compsky::mysql::assign_next_row(RES1, &ROW1, &regexpr_str)){
		// Only one iteration - so the memory is never freed, so the reason_name2id names remain valid.
		*compsky::regex::convert_named_groups(regexpr_str + 1,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents) = 0;
		// Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst

		regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str,  boost::regex::perl | boost::regex::optimize);
	}
}

void init_regexp_from_file(){
	std::vector<compsky::regex::CharPaar> reason_name2id;
	std::vector<int> groupindx2reason;
	std::vector<bool> record_contents;
	
	init_regexp_from_file(reason_name2id, groupindx2reason, record_contents);
	char* dummy;
	compsky::mysql::assign_next_row(RES1, &ROW1, &dummy); // Second call, which will free the results as it is obvious we do not care for them.
}

}
