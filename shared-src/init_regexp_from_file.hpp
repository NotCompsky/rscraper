#ifndef __INIT_REGEXP_FROM_FILE_H
#define __INIT_REGEXP_FROM_FILE_H

#include <boost/regex.hpp>
#include <vector>


namespace filter_comment_body {


extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;



void init_regexp_from_file(std::vector<char*>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents);


}

#endif
