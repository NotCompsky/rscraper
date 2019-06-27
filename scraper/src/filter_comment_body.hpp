/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef __FILTER_COMMENT_BODY__
#define __FILTER_COMMENT_BODY__

#include "structs.h" // for cmnt_meta

#ifdef USE_BOOST_REGEX
# include <boost/regex.hpp> // for boost::
#endif


namespace filter_comment_body {

extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;

unsigned int match(struct cmnt_meta metadata, const char* str, const int str_len, bool& to_record_contents);

} // end namespace

#endif
