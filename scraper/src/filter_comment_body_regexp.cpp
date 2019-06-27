/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

/*
Multiple groups can share the same group name.

If there are named groups in the regex that are not registered in the database, they are added to the reason_matched table.

It cannot end in a literal newline. If such is desired, use [\n]
*/

#ifdef USE_BOOST_REGEX

#include "filter_comment_body_regexp.hpp"

#ifdef FS_EXPERIMENTAL
# include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#else
# include <filesystem>
namespace fs = std::filesystem;
#endif

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>
#include <compsky/regex/named_groups.hpp>

#include "filter_comment_body.hpp"
#include "error_codes.hpp" // for myerr:*


extern MYSQL_RES* RES;
extern MYSQL_ROW ROW;


namespace filter_comment_body {


std::vector<char*> reason_name2id;
std::vector<int> groupindx2reason;
std::vector<bool> record_contents;

std::vector<std::vector<uint64_t>> SUBREDDIT_WHITELISTS;
std::vector<std::vector<uint64_t>> SUBREDDIT_BLACKLISTS;


void populate_reason2name(){
    compsky::mysql::query_buffer(&RES, "SELECT r.id, r.name, IFNULL(w.subreddit,0), IFNULL(b.subreddit,0) FROM reason_matched r LEFT JOIN reason_subreddit_blacklist b ON r.id=b.reason LEFT JOIN reason_subreddit_whitelist w ON r.id=w.reason ORDER BY r.id DESC");
    int reason_id;
    char* name;
    uint64_t subreddit_wl, subreddit_bl;
    constexpr static const compsky::mysql::flag::SizeOfAssigned f;
    size_t name_sz;
    if(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, f, &name_sz, &name, &subreddit_wl, &subreddit_bl)){
        // Initialise the vectors
        reason_name2id.reserve(reason_id);
        SUBREDDIT_WHITELISTS.reserve(reason_id);
        SUBREDDIT_BLACKLISTS.reserve(reason_id);
        for (auto i = 0;  i < reason_id + 1;  ++i){
            SUBREDDIT_WHITELISTS.emplace_back();
            SUBREDDIT_BLACKLISTS.emplace_back();
            reason_name2id.push_back(nullptr);
        }
    }
    do {
        char* dummy = (char*)malloc(name_sz + 1); // Allow for terminating null byte
        if (reason_name2id[reason_id] == nullptr){
            if (dummy == nullptr)
                exit(myerr::OUT_OF_MEMORY);
            memcpy(dummy,  name,  name_sz + 1);
            reason_name2id[reason_id] = dummy;
        }
        if (subreddit_wl != 0)
            SUBREDDIT_WHITELISTS[reason_id].push_back(subreddit_wl);
        if (subreddit_bl != 0)
            SUBREDDIT_BLACKLISTS[reason_id].push_back(subreddit_bl);
    } while(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, f, &name_sz, &name, &subreddit_wl, &subreddit_bl));
}

void init(){
    const char* fp = getenv("RSCRAPER_REGEX_FILE");
    if (fp == nullptr)
        return;
    
    size_t f_sz = fs::file_size(fp);
    void* dummy = malloc(f_sz + 2); // 1 for blank at beginning, 1 for terminating \0
    if (dummy == nullptr)
        exit(myerr::OUT_OF_MEMORY);
    char* regexpr_str = (char*)dummy;
    
    FILE* f = fopen(fp, "rb");
    fread(regexpr_str + 1,  1,  f_sz,  f);
    regexpr_str[f_sz + 1] = 0; // Ensure there is a terminating \0
    
    populate_reason2name();
    
    char* regexpr_str_end = compsky::regex::convert_named_groups(regexpr_str + 1,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents);
    // Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst
    
    if (*(regexpr_str_end - 1) == '\n')
        // Very confused what is happening here, but it seems that some files have \n appended to them by fread, while others do not.
        // A small test file containing only `(?P<test>a)` written in `vim` is given a trailing \n by fread
        // while a larger file containing newlines elsewhere but not at the end is not given a trailing \n by fread
        *(regexpr_str_end - 1) = 0;
    else *regexpr_str_end = 0;
    
    constexpr static const compsky::asciify::flag::ChangeBuffer chbuf;
    constexpr static const compsky::asciify::flag::Escape esc;
    
    compsky::asciify::asciify(chbuf, compsky::asciify::BUF, 0, "INSERT IGNORE INTO reason_matched (id,name) VALUES");
    
    for (auto i = 0;  i < reason_name2id.size();  ++i)
        compsky::asciify::asciify("(", i, ",\"", esc, '"', reason_name2id[i], "\"),");
    
    compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Ignore trailing comma
    
    regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str,  boost::regex::perl | boost::regex::optimize);
    
    free(regexpr_str);
}


}
#endif
