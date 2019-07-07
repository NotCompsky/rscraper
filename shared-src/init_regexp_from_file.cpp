#include "init_regexp_from_file.hpp"

#include <compsky/regex/named_groups.hpp>

#ifdef FS_EXPERIMENTAL
# include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#else
# include <filesystem>
namespace fs = std::filesystem;
#endif


namespace filter_comment_body {

boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;


void init_regexp_from_file(std::vector<char*>& reason_name2id,  std::vector<int>& groupindx2reason,  std::vector<bool>& record_contents){
    const char* fp = getenv("RSCRAPER_REGEX_FILE");
    if (fp == nullptr)
        return;
    
    size_t f_sz = fs::file_size(fp);
    void* dummy = malloc(f_sz + 2); // 1 for blank at beginning, 1 for terminating \0
    if (dummy == nullptr)
        exit(1);
    char* regexpr_str = (char*)dummy;
    
    FILE* f = fopen(fp, "rb");
    fread(regexpr_str + 1,  1,  f_sz,  f);
    fclose(f);
    regexpr_str[f_sz + 1] = 0; // Ensure there is a terminating \0
    
    char* regexpr_str_end = compsky::regex::convert_named_groups(regexpr_str + 1,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents);
    // Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst
    
    if (*(regexpr_str_end - 1) == '\n')
        // Very confused what is happening here, but it seems that some files have \n appended to them by fread, while others do not.
        // A small test file containing only `(?P<test>a)` written in `vim` is given a trailing \n by fread
        // while a larger file containing newlines elsewhere but not at the end is not given a trailing \n by fread
        *(regexpr_str_end - 1) = 0;
    else *regexpr_str_end = 0;
    
    regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str,  boost::regex::perl | boost::regex::optimize);
    
    free(regexpr_str);
}

}
