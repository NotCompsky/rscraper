#ifndef RSCRAPER_SHARED_SRC_INIT_REGEXP_FROM_FILE_HPP
#define RSCRAPER_SHARED_SRC_INIT_REGEXP_FROM_FILE_HPP


#include <compsky/regex/named_groups.hpp>

#include <boost/regex.hpp>
#include <vector>


namespace filter_comment_body {


extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;



void init_regexp_from_file(std::vector<compsky::regex::CharPaar>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents);

void init_regexp_from_file();


}

#endif
