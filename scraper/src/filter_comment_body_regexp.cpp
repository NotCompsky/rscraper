/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

/*
Multiple groups can share the same group name.

If there are named groups in the regex that are not registered in the database, they are added to the reason_matched table.
*/

#include "filter_comment_body_regexp.hpp"

#ifdef FS_EXPERIMENTAL
# include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;
#else
# include <filesystem>
namespace fs = std::filesystem;
#endif

#ifdef DEBUG
# include <execinfo.h> // for backtracing
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

char* regexpr_str;

std::vector<std::vector<uint64_t>> SUBREDDIT_BLACKLISTS;

void populate_reason2name(){
    compsky::mysql::query_buffer(&RES, "SELECT r.id, r.name, IFNULL(b.subreddit,0) FROM reason_matched r LEFT JOIN reason_subreddit_blacklist b ON r.id=b.reason ORDER BY r.id DESC");
    int reason_id;
    char* name;
    uint64_t subreddit_id;
    if(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, &name, &subreddit_id)){
        // Initialise the vectors
        reason_name2id.reserve(reason_id);
        SUBREDDIT_BLACKLISTS.reserve(reason_id);
        for (auto i = 0;  i < reason_id + 1;  ++i){
            SUBREDDIT_BLACKLISTS.emplace_back();
            reason_name2id.emplace_back();
        }
    }
    do {
        reason_name2id[reason_id] = name;
        SUBREDDIT_BLACKLISTS[reason_id].push_back(subreddit_id);
    } while(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, &name, &subreddit_id));
}

void init(){
    const char* fp = getenv("RSCRAPER_REGEX_FILE");
    if (fp == nullptr)
        return;
    
    size_t f_sz = fs::file_size(fp);
    void* dummy = malloc(f_sz + 2); // 1 for blank at beginning, 1 for \0 at end
    if (dummy == nullptr)
        exit(myerr::OUT_OF_MEMORY);
    regexpr_str = (char*)dummy;
    
    FILE* f = fopen(fp, "rb");
    fread(regexpr_str + 1,  1,  f_sz,  f);
    regexpr_str[1 + f_sz] = 0; // Terminate
    
    populate_reason2name();
    
    compsky::regex::convert_named_groups(regexpr_str + 1,  regexpr_str,  reason_name2id,  groupindx2reason);
    // Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst
    
    constexpr static const compsky::asciify::flag::ChangeBuffer chbuf;
    constexpr static const compsky::asciify::flag::Escape esc;
    
    compsky::asciify::asciify(chbuf, compsky::asciify::BUF, 0, "INSERT IGNORE INTO reason_matched (id,name) VALUES");
    
    for (auto i = 0;  i < reason_name2id.size();  ++i)
        compsky::asciify::asciify("(", i, ",\"", esc, '"', reason_name2id[i], "\"),");
    
    compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Ignore trailing comma
    
    regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(regexpr_str, boost::regex::perl);
}


}
