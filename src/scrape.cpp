/*
 * Scrape comments from /r/all as they appear
 */


#include <string.h> // for memcpy
#include <unistd.h> // for sleep
#include <string.h> // for malloc, realloc

#include "rapidjson_utils.h" // for SET_DBG_* macros

#include "error_codes.hpp" // for myerr:*

#include "reddit_utils.hpp" // for myru::*
#include "curl_utils.hpp" // for mycu::*
#include "redditcurl_utils.hpp" // for myrcu::*, rapidjson::*

#include "filter_comment_body.cpp" // for filter_comment_body::*
#include "filter_user.cpp" // for filter_user::*
#include "filter_subreddit.cpp" // for filter_subreddit::*

#include "mymysql.hpp" // for mymysql::*, BUF, BUF_INDX

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}


#ifdef DEBUG
  #define PRINTF(...) printf(__VA_ARGS__);
#else
  #define PRINTF(...) ;
#endif


char* BUF = (char*)malloc(4096);
int BUF_SZ = 4096;
int BUF_INDX = 0;


constexpr const char* SQL__INSERT_SUBMISSION_FROM_CMNT_STR = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
char SQL__INSERT_SUBMISSION_FROM_CMNT[strlen(SQL__INSERT_SUBMISSION_FROM_CMNT_STR) + 100*strlen("(01234567890123456789,01234567890123456789,2),") + 1] = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
int SQL__INSERT_SUBMISSION_FROM_CMNT_INDX;




constexpr const char* SQL__INSERT_INTO_USER2SUBCNT_STR = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
char SQL__INSERT_INTO_USER2SUBCNT[strlen(SQL__INSERT_INTO_USER2SUBCNT_STR) + 100*(1 + 1+1+20+1+20 + 1 + 1) + 1] = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
int SQL__INSERT_INTO_USER2SUBCNT_INDX;


constexpr const char* SQL__INSERT_INTO_SUBREDDIT_STR = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
char SQL__INSERT_INTO_SUBREDDIT[strlen(SQL__INSERT_INTO_SUBREDDIT_STR) + 100*(1 + 20+1+128 + 1 + 1) + 1] = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
int SQL__INSERT_INTO_SUBREDDIT_INDX;


void count_user_subreddit_cmnt(const unsigned long int user_id,  const unsigned long int subreddit_id, const char* subreddit_name){
    char* dummy = BUF;
    auto dummy_indx = BUF_INDX;
    
    BUF = SQL__INSERT_INTO_USER2SUBCNT;
    BUF_INDX = SQL__INSERT_INTO_USER2SUBCNT_INDX;
    asciify("(1,",  user_id,  ',',  subreddit_id,  "),");
    SQL__INSERT_INTO_USER2SUBCNT_INDX = BUF_INDX;
    
    
    BUF = SQL__INSERT_INTO_SUBREDDIT;
    BUF_INDX = SQL__INSERT_INTO_SUBREDDIT_INDX;
    asciify("(",  subreddit_id,  ",\"",  subreddit_name,  "\"),");
    SQL__INSERT_INTO_SUBREDDIT_INDX = BUF_INDX;
    
    BUF = dummy;
    BUF_INDX = dummy_indx;
}

void process_live_cmnt(rapidjson::Value& cmnt, const unsigned long int cmnt_id){
    SET_STR(body,           cmnt["data"]["body"]);
    SET_STR(subreddit_name, cmnt["data"]["subreddit"]);
    SET_STR(author_name,    cmnt["data"]["author"]);
    
    
    const unsigned long int author_id = myru::id2n_lower(cmnt["data"]["author_fullname"].GetString() + 3); // Skip "t2_" prefix
    const unsigned long int subreddit_id = myru::id2n_lower(cmnt["data"]["subreddit_id"].GetString() + 3); // Skip "t3_" prefix
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
    
    mymysql::exec("INSERT IGNORE INTO user (id, name) VALUES (",  author_id,  ",'",  author_name,  "')");
    
    unsigned long int parent_id = myru::id2n_lower(cmnt["data"]["parent_id"].GetString() + 3);
    unsigned long int submission_id;
    
    if (cmnt["data"]["parent_id"].GetString()[1] == '3'){
        // "t3_" or "t1_" prefix
        submission_id = parent_id;
        parent_id = 0;
    } else {
        submission_id = myru::id2n_lower(cmnt["data"]["link_id"].GetString() + 3);
    }
    
    const char* cmnt_content = cmnt["data"]["body"].GetString();
    
    
    StartConcatWithCommaFlag a;
    EndConcatWithCommaFlag b;
    
    StartConcatWithApostrapheAndCommaFlag c;
    EndConcatWithApostrapheAndCommaFlag d;
    
    mymysql::exec("INSERT IGNORE INTO comment (id, parent_id, author_id, submission_id, created_at, reason_matched, content) values(",  a,  cmnt_id,  parent_id,  author_id,  submission_id,  created_at,  reason_matched,  b,  ',',  c,  cmnt_content,  d,  ")");
    
    
    /*
    Checks if a submission entry exists, and if not, creates one (but is based only on the information visible from a comment entry)
    */
    char* dummy = BUF;
    auto dummy_indx = BUF_INDX;
    
    BUF = SQL__INSERT_SUBMISSION_FROM_CMNT;
    BUF_INDX = SQL__INSERT_SUBMISSION_FROM_CMNT_INDX;
    asciify("(",  submission_id,  ',',  subreddit_id,  ',',  '0' + is_submission_nsfw,  "),");
    SQL__INSERT_SUBMISSION_FROM_CMNT_INDX = BUF_INDX;
    
    BUF = dummy;
    BUF_INDX = dummy_indx;
}

unsigned long int process_live_replies(rapidjson::Value& replies, const unsigned long int last_processed_cmnt_id){
    /*
    'replies' object is the 'replies' JSON object which has property 'kind' of value 'Listing'
    */
    unsigned long int cmnt_id;
    int i = 0;
    
    SQL__INSERT_SUBMISSION_FROM_CMNT_INDX = strlen(SQL__INSERT_SUBMISSION_FROM_CMNT_STR);
    SQL__INSERT_INTO_USER2SUBCNT_INDX = strlen(SQL__INSERT_INTO_USER2SUBCNT_STR);
    SQL__INSERT_INTO_SUBREDDIT_INDX = strlen(SQL__INSERT_INTO_SUBREDDIT_STR);
    
    for (rapidjson::Value::ValueIterator itr = replies["data"]["children"].Begin();  itr != replies["data"]["children"].End();  ++itr){
        cmnt_id = myru::id2n_lower((*itr)["data"]["id"].GetString()); // No "t1_" prefix
        if (cmnt_id <= last_processed_cmnt_id)
            // Not '==' since it is possible for comments to have been deleted between calls
            break;
        process_live_cmnt(*itr, cmnt_id);
        ++i;
    }
    
    if (SQL__INSERT_SUBMISSION_FROM_CMNT_INDX != strlen(SQL__INSERT_SUBMISSION_FROM_CMNT_STR)){
        SQL__INSERT_SUBMISSION_FROM_CMNT[--SQL__INSERT_SUBMISSION_FROM_CMNT_INDX] = 0; // Overwrite trailing comma
        PRINTF("stmt: %s\n", SQL__INSERT_SUBMISSION_FROM_CMNT);
        mymysql::exec(SQL__INSERT_SUBMISSION_FROM_CMNT);
    }
    
    if (SQL__INSERT_INTO_USER2SUBCNT_INDX != strlen(SQL__INSERT_INTO_USER2SUBCNT_STR)){
        --SQL__INSERT_INTO_USER2SUBCNT_INDX; // Overwrite trailing comma
        constexpr const char* b = " ON DUPLICATE KEY UPDATE count = count + 1";
        memcpy(SQL__INSERT_INTO_USER2SUBCNT + SQL__INSERT_INTO_USER2SUBCNT_INDX,  b,  strlen(b) + 1);
        PRINTF("stmt: %s\n", SQL__INSERT_INTO_USER2SUBCNT);
        mymysql::exec(SQL__INSERT_INTO_USER2SUBCNT);
    }
    
    if (SQL__INSERT_INTO_SUBREDDIT_INDX != strlen(SQL__INSERT_INTO_SUBREDDIT_STR)){
        SQL__INSERT_INTO_SUBREDDIT[--SQL__INSERT_INTO_SUBREDDIT_INDX] = 0;
        PRINTF("stmt: %s\n", SQL__INSERT_INTO_SUBREDDIT);
        mymysql::exec(SQL__INSERT_INTO_SUBREDDIT);
    }
    
    return myru::id2n_lower(replies["data"]["children"][0]["data"]["id"].GetString());
}

void process_all_comments_live(){
    unsigned long int last_processed_cmnt_id = 0;
    
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
    mymysql::init(argv[1]);  // Init SQL
    mycu::init();         // Init CURL
    myrcu::init(argv[2]); // Init OAuth
    
    process_all_comments_live();
}
