/*
 * Scrape comments from /r/all as they appear
 */


#include <string.h> // for memcpy
#ifdef _WIN32
  #include <windows.h> // for sleep
  #define sleep(n) Sleep(1000 * n)
#else
  #include <unistd.h> // for sleep
#endif
#include <string.h> // for malloc, realloc

#include "rapidjson_utils.h" // for SET_DBG_* macros

#include "error_codes.hpp" // for myerr:*

#include "reddit_utils.hpp" // for myru::*
#include "curl_utils.hpp" // for mycu::*
#include "redditcurl_utils.hpp" // for myrcu::*, rapidjson::*

#include "filter_comment_body.cpp" // for filter_comment_body::*
#include "filter_user.cpp" // for filter_user::*
#include "filter_subreddit.cpp" // for filter_subreddit::*

#include <compsky/asciify/base.hpp>
#include <compsky/mysql/query.hpp>


#ifdef DEBUG
  #define PRINTF(...) printf(__VA_ARGS__);
#else
  #define PRINTF(...) ;
#endif


MYSQL_ROW ROW;

namespace compsky {
    namespace asciify {
        char* BUF = (char*)malloc(4096);
    }
}


constexpr size_t strlen_constexpr(const char* s){
    // GCC strlen is constexpr; this is apparently a bug
    return *s  ?  1 + strlen_constexpr(s + 1)  :  0;
}

constexpr const char* SQL__INSERT_SUBMISSION_FROM_CMNT_STR = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
char SQL__INSERT_SUBMISSION_FROM_CMNT[strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR) + 100*strlen_constexpr("(01234567890123456789,01234567890123456789,2),") + 1] = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
size_t SQL__INSERT_SUBMISSION_FROM_CMNT_INDX;




constexpr const char* SQL__INSERT_INTO_USER2SUBCNT_STR = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
char SQL__INSERT_INTO_USER2SUBCNT[strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR) + 100*(1 + 1+1+20+1+20 + 1 + 1) + 1] = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
size_t SQL__INSERT_INTO_USER2SUBCNT_INDX;


constexpr const char* SQL__INSERT_INTO_SUBREDDIT_STR = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
char SQL__INSERT_INTO_SUBREDDIT[strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR) + 100*(1 + 20+1+128 + 1 + 1) + 1] = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
size_t SQL__INSERT_INTO_SUBREDDIT_INDX;


void count_user_subreddit_cmnt(const uint64_t user_id,  const uint64_t subreddit_id, const char* subreddit_name){
    char* dummy = compsky::asciify::BUF;
    auto dummy_indx = compsky::asciify::BUF_INDX;
    
    compsky::asciify::BUF = SQL__INSERT_INTO_USER2SUBCNT;
    compsky::asciify::BUF_INDX = SQL__INSERT_INTO_USER2SUBCNT_INDX;
    compsky::asciify::asciify("(1,",  user_id,  ',',  subreddit_id,  "),");
    SQL__INSERT_INTO_USER2SUBCNT_INDX = compsky::asciify::BUF_INDX;
    
    
    compsky::asciify::BUF = SQL__INSERT_INTO_SUBREDDIT;
    compsky::asciify::BUF_INDX = SQL__INSERT_INTO_SUBREDDIT_INDX;
    compsky::asciify::asciify("(",  subreddit_id,  ",\"",  subreddit_name,  "\"),");
    SQL__INSERT_INTO_SUBREDDIT_INDX = compsky::asciify::BUF_INDX;
    
    compsky::asciify::BUF = dummy;
    compsky::asciify::BUF_INDX = dummy_indx;
}

void process_live_cmnt(rapidjson::Value& cmnt, const uint64_t cmnt_id){
    SET_STR(body,           cmnt["data"]["body"]);
    SET_STR(subreddit_name, cmnt["data"]["subreddit"]);
    SET_STR(author_name,    cmnt["data"]["author"]);
    
    
    const uint64_t author_id = myru::id2n_lower(cmnt["data"]["author_fullname"].GetString() + 3); // Skip "t2_" prefix
    const uint64_t subreddit_id = myru::id2n_lower(cmnt["data"]["subreddit_id"].GetString() + 3); // Skip "t3_" prefix
    const bool is_submission_nsfw = cmnt["data"]["over_18"].GetBool();
    char is_subreddit_nsfw = 2; // 0 for certainly SFW, 1 for certainly NSFW. 2 for unknown.
    
    if (!is_submission_nsfw)
        is_subreddit_nsfw = 0;
    
    
    if (filter_subreddit::to_count(subreddit_id) && filter_user::to_count(author_id))
        count_user_subreddit_cmnt(author_id, subreddit_id, subreddit_name);
    
    
    struct cmnt_meta metadata = {
        author_name,
        subreddit_name,
        
        author_id,
        subreddit_id,
    };
    
    
    switch (filter_user::matches_id(author_id)){
        case 1: return; // blacklist
        case 2: goto goto__do_process_this_live_cmnt; // whitelist
        default: break; // 0 is the default
    }
    
    switch (filter_subreddit::matches_id(subreddit_id)){
        case 1: return; // blacklist
        case 2: goto goto__do_process_this_live_cmnt; // whitelist
        default: break; // 0 is the default
    }
    
    
    unsigned int reason_matched;
    if ((reason_matched = filter_comment_body::wl::match(metadata, body, strlen(body)))){
        PRINTF("MATCHED: %s\n", filter_comment_body::wl::what[0].str().c_str());
        goto goto__do_process_this_live_cmnt;
    }
    // if filter_comment_body::bl: return;
    
    
    
    return;
    
    goto__do_process_this_live_cmnt:
    
    
    SET_STR(permalink,      cmnt["data"]["permalink"]);
    
    const time_t RSET_FLT(created_at,     cmnt["data"]["created_utc"]);
    
    compsky::mysql::exec("INSERT IGNORE INTO user (id, name) VALUES (",  author_id,  ",'",  author_name,  "')");
    
    uint64_t parent_id = myru::id2n_lower(cmnt["data"]["parent_id"].GetString() + 3);
    uint64_t submission_id;
    
    if (cmnt["data"]["parent_id"].GetString()[1] == '3'){
        // "t3_" or "t1_" prefix
        submission_id = parent_id;
        parent_id = 0;
    } else {
        submission_id = myru::id2n_lower(cmnt["data"]["link_id"].GetString() + 3);
    }
    
    const char* cmnt_content = cmnt["data"]["body"].GetString();
    
    
    constexpr static const compsky::asciify::flag::concat::Start a;
    constexpr static const compsky::asciify::flag::concat::End b;
    constexpr static const compsky::asciify::flag::Escape esc;
    
    compsky::mysql::exec("INSERT IGNORE INTO comment (id, parent_id, author_id, submission_id, created_at, reason_matched, content) values(",  a, ',',  cmnt_id,  parent_id,  author_id,  submission_id,  created_at,  reason_matched,  b,  ",\"",  esc,  '"',  cmnt_content,  "\")");
    
    
    /*
    Checks if a submission entry exists, and if not, creates one (but is based only on the information visible from a comment entry)
    */
    char* dummy = compsky::asciify::BUF;
    auto dummy_indx = compsky::asciify::BUF_INDX;
    
    compsky::asciify::BUF = SQL__INSERT_SUBMISSION_FROM_CMNT;
    compsky::asciify::BUF_INDX = SQL__INSERT_SUBMISSION_FROM_CMNT_INDX;
    compsky::asciify::asciify("(",  submission_id,  ',',  subreddit_id,  ',',  '0' + is_submission_nsfw,  "),");
    SQL__INSERT_SUBMISSION_FROM_CMNT_INDX = compsky::asciify::BUF_INDX;
    
    compsky::asciify::BUF = dummy;
    compsky::asciify::BUF_INDX = dummy_indx;
}

uint64_t process_live_replies(rapidjson::Value& replies, const uint64_t last_processed_cmnt_id){
    /*
    'replies' object is the 'replies' JSON object which has property 'kind' of value 'Listing'
    */
    uint64_t cmnt_id;
    
    SQL__INSERT_SUBMISSION_FROM_CMNT_INDX = strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR);
    SQL__INSERT_INTO_USER2SUBCNT_INDX = strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR);
    SQL__INSERT_INTO_SUBREDDIT_INDX = strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR);
    
    for (rapidjson::Value::ValueIterator itr = replies["data"]["children"].Begin();  itr != replies["data"]["children"].End();  ++itr){
        cmnt_id = myru::id2n_lower((*itr)["data"]["id"].GetString()); // No "t1_" prefix
        if (cmnt_id <= last_processed_cmnt_id)
            // Not '==' since it is possible for comments to have been deleted between calls
            break;
        process_live_cmnt(*itr, cmnt_id);
    }
    
    if (SQL__INSERT_SUBMISSION_FROM_CMNT_INDX != strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR)){
        SQL__INSERT_SUBMISSION_FROM_CMNT[--SQL__INSERT_SUBMISSION_FROM_CMNT_INDX] = 0; // Overwrite trailing comma
        PRINTF("stmt: %s\n", SQL__INSERT_SUBMISSION_FROM_CMNT);
        compsky::mysql::exec_buffer(SQL__INSERT_SUBMISSION_FROM_CMNT, SQL__INSERT_SUBMISSION_FROM_CMNT_INDX);
    }
    
    if (SQL__INSERT_INTO_USER2SUBCNT_INDX != strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR)){
        --SQL__INSERT_INTO_USER2SUBCNT_INDX; // Overwrite trailing comma
        constexpr const char* b = " ON DUPLICATE KEY UPDATE count = count + 1";
        memcpy(SQL__INSERT_INTO_USER2SUBCNT + SQL__INSERT_INTO_USER2SUBCNT_INDX,  b,  strlen_constexpr(b));
        SQL__INSERT_INTO_USER2SUBCNT_INDX += strlen_constexpr(b);
        PRINTF("stmt: %s\n", SQL__INSERT_INTO_USER2SUBCNT);
        compsky::mysql::exec_buffer(SQL__INSERT_INTO_USER2SUBCNT, SQL__INSERT_INTO_USER2SUBCNT_INDX);
    }
    
    if (SQL__INSERT_INTO_SUBREDDIT_INDX != strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR)){
        SQL__INSERT_INTO_SUBREDDIT[--SQL__INSERT_INTO_SUBREDDIT_INDX] = 0;
        PRINTF("stmt: %s\n", SQL__INSERT_INTO_SUBREDDIT);
        compsky::mysql::exec_buffer(SQL__INSERT_INTO_SUBREDDIT, SQL__INSERT_INTO_SUBREDDIT_INDX);
    }
    
    return myru::id2n_lower(replies["data"]["children"][0]["data"]["id"].GetString());
}

void process_all_comments_live(){
    uint64_t last_processed_cmnt_id = 0;
    
    while (true){
        sleep(myrcu::REDDIT_REQUEST_DELAY);
        
        
        mycu::request(myrcu::API_ALLCOMMENTS_URL);
        
        rapidjson::Document d;
        
        if (myrcu::try_again(d))
            continue;
        
        last_processed_cmnt_id = process_live_replies(d, last_processed_cmnt_id);
    }
}

int main(const int argc, const char* argv[]){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    mycu::init();         // Init CURL
    myrcu::init(getenv("RSCRAPER_REDDIT_CFG")); // Init OAuth
    
    process_all_comments_live();
    
    compsky::mysql::exit();
}
