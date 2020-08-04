/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "error_codes.hpp" // for myerr:*

#include "curl_utils.hpp" // for mycu::*
#include "id2str.hpp" // for str2id
#include "redditcurl_utils.hpp" // for myrcu::*, rapidjson::*

#include "filter_comment_body.hpp" // for filter_comment_body::*
#include "filter_comment_body_regexp.hpp"
#include "filter_subreddit.hpp" // for filter_subreddit::*
#include "filter_user.hpp" // for filter_user::*

#include <compsky/asciify/asciify.hpp>
#include <compsky/mysql/query.hpp>

#include <cstring> // for malloc, memcpy
#ifdef _WIN32
# include <windows.h> // for sleep
# define sleep(n) Sleep(1000 * n)
#else
# include <unistd.h> // for sleep
#endif


MYSQL_RES* RES1;
MYSQL_ROW ROW1;

char BUF[81000];
char* ITR = BUF;

namespace _mysql {
	MYSQL* obj;
	MYSQL_RES* res1;
	MYSQL_ROW row1;
	constexpr static const size_t buf_sz = 512;
	char buf[buf_sz];
	char* auth[6];
}


constexpr size_t strlen_constexpr(const char* s){
	// GCC strlen is constexpr; this is apparently a bug
	return (*s != 0)  ?  1 + strlen_constexpr(s + 1)  :  0;
}

constexpr static const char* SQL__INSERT_SUBMISSION_FROM_CMNT_STR = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
char SQL__INSERT_SUBMISSION_FROM_CMNT[strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR) + 100*strlen_constexpr("(01234567890123456789,01234567890123456789,2),") + 1] = "INSERT IGNORE INTO submission (id, subreddit_id, nsfw) values";
size_t SQL__INSERT_SUBMISSION_FROM_CMNT_INDX;




constexpr static const char* SQL__INSERT_INTO_USER2SUBCNT_STR = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
char SQL__INSERT_INTO_USER2SUBCNT[strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR) + 100*(1 + 1+1+20+1+20 + 1 + 1) + 1] = "INSERT INTO user2subreddit_cmnt_count (count, user_id, subreddit_id) VALUES ";
size_t SQL__INSERT_INTO_USER2SUBCNT_INDX;


constexpr const char* SQL__INSERT_INTO_SUBREDDIT_STR = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
char SQL__INSERT_INTO_SUBREDDIT[strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR) + 100*(1 + 20+1+128 + 1 + 1) + 1] = "INSERT IGNORE INTO subreddit (id, name) VALUES ";
size_t SQL__INSERT_INTO_SUBREDDIT_INDX;


bool contains(uint64_t* list,  uint64_t item){
	while(*list != 0){
		if (*list == item)
			return true;
		++list;
	}
	return false;
}


template<typename... Args>
void exec(Args... args){
	compsky::mysql::exec(_mysql::obj, BUF, args...);
}
template<typename... Args>
void exec_buffer(Args... args){
	compsky::mysql::exec_buffer(_mysql::obj, args...);
}


void count_user_subreddit_cmnt(const uint64_t user_id,  const uint64_t subreddit_id, const char* subreddit_name){
	compsky::asciify::asciify(SQL__INSERT_INTO_USER2SUBCNT,  &SQL__INSERT_INTO_USER2SUBCNT_INDX,  "(1,",  user_id,  ',',  subreddit_id,  "),");
	compsky::asciify::asciify(SQL__INSERT_INTO_SUBREDDIT,  &SQL__INSERT_INTO_SUBREDDIT_INDX,  "(",  subreddit_id,  ",\"",  subreddit_name,  "\"),");
}

void process_this_comment(const rapidjson::Value& cmnt,  const uint64_t author_id,  const char* author_name,  const uint64_t cmnt_id,  const uint64_t subreddit_id,  const unsigned int reason_matched,  const bool is_submission_nsfw,  const bool to_record_contents){
	const time_t created_at = cmnt["data"]["created_utc"].GetFloat(); // It's delivered in float format
	
	exec("INSERT IGNORE INTO user (id, name) VALUES (",  author_id,  ",'",  author_name,  "')");
	
	uint64_t parent_id = str2id(cmnt["data"]["parent_id"].GetString() + 3);
	uint64_t submission_id;
	
	if (cmnt["data"]["parent_id"].GetString()[1] == '3'){
		// "t3_" or "t1_" prefix
		submission_id = parent_id;
		parent_id = 0;
	} else {
		submission_id = str2id(cmnt["data"]["link_id"].GetString() + 3);
	}
	
	const char* cmnt_content = (to_record_contents)  ?  cmnt["data"]["body"].GetString()  :  "";
	
	
	constexpr static const compsky::asciify::flag::concat::Start a;
	constexpr static const compsky::asciify::flag::concat::End b;
	constexpr static const compsky::asciify::flag::Escape esc;
	
	exec("INSERT IGNORE INTO comment (id, parent_id, author_id, submission_id, created_at, reason_matched, content) values(",  a, ',',  cmnt_id,  parent_id,  author_id,  submission_id,  created_at,  reason_matched,  b,  ",\"",  esc,  '"',  cmnt_content,  "\")");
	
	
	/*
	Checks if a submission entry exists, and if not, creates one (but is based only on the information visible from a comment entry)
	*/
	compsky::asciify::asciify(SQL__INSERT_SUBMISSION_FROM_CMNT,  &SQL__INSERT_SUBMISSION_FROM_CMNT_INDX,  "(",  submission_id,  ',',  subreddit_id,  ',',  '0' + is_submission_nsfw,  "),");
}

void process_live_cmnt(const rapidjson::Value& cmnt,  const uint64_t cmnt_id){
	const char* body            = cmnt["data"]["body"].GetString();
	const char* subreddit_name  = cmnt["data"]["subreddit"].GetString();
	const char* author_name     = cmnt["data"]["author"].GetString();
	
	const uint64_t author_id = str2id(cmnt["data"]["author_fullname"].GetString() + 3); // Skip "t2_" prefix
	const uint64_t subreddit_id = str2id(cmnt["data"]["subreddit_id"].GetString() + 3); // Skip "t3_" prefix
	const bool is_submission_nsfw = cmnt["data"]["over_18"].GetBool();
	const uint8_t is_subreddit_nsfw = (is_submission_nsfw) ? 2 : 0; // 0 for certainly SFW, 1 for certainly NSFW. 2 for unknown. // TODO: Record this
	
	if (!contains(filter_subreddit::BLACKLIST_COUNT, subreddit_id)  &&  !contains(filter_user::BLACKLIST_COUNT, author_id))
		count_user_subreddit_cmnt(author_id, subreddit_id, subreddit_name);
	
	
	struct cmnt_meta metadata = {
		author_name,
		subreddit_name,
		
		author_id,
		subreddit_id,
	};
	
	unsigned int reason_matched = 0;
	
	if (contains(filter_user::WHITELIST_BODY, author_id))
		return process_this_comment(cmnt, author_id, author_name, cmnt_id, subreddit_id, reason_matched, is_submission_nsfw, true);
	if (contains(filter_user::BLACKLIST_BODY, author_id))
		return;
	
	if (contains(filter_subreddit::WHITELIST_BODY, subreddit_id))
		return process_this_comment(cmnt, author_id, author_name, cmnt_id, subreddit_id, reason_matched, is_submission_nsfw, true);
	if (contains(filter_subreddit::BLACKLIST_BODY, subreddit_id))
		return;
	
	if ((reason_matched = filter_comment_body::match(metadata, body, strlen(body))))
		return process_this_comment(cmnt, author_id, author_name, cmnt_id, subreddit_id, reason_matched, is_submission_nsfw, filter_comment_body::record_contents[reason_matched]);
	
	return;
}

uint64_t process_live_replies(rapidjson::Value& replies,  const uint64_t last_processed_cmnt_id){
	/*
	'replies' object is the 'replies' JSON object which has property 'kind' of value 'Listing'
	*/
	uint64_t cmnt_id;
	
	SQL__INSERT_SUBMISSION_FROM_CMNT_INDX = strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR);
	SQL__INSERT_INTO_USER2SUBCNT_INDX = strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR);
	SQL__INSERT_INTO_SUBREDDIT_INDX = strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR);
	
	for (rapidjson::Value::ValueIterator itr = replies["data"]["children"].Begin();  itr != replies["data"]["children"].End();  ++itr){
		cmnt_id = str2id((*itr)["data"]["id"].GetString()); // No "t1_" prefix
		if (cmnt_id <= last_processed_cmnt_id)
			// Not '==' since it is possible for comments to have been deleted between calls
			break;
		process_live_cmnt(*itr, cmnt_id);
	}
	
	if (SQL__INSERT_SUBMISSION_FROM_CMNT_INDX != strlen_constexpr(SQL__INSERT_SUBMISSION_FROM_CMNT_STR)){
		SQL__INSERT_SUBMISSION_FROM_CMNT[--SQL__INSERT_SUBMISSION_FROM_CMNT_INDX] = 0; // Overwrite trailing comma
		exec_buffer(SQL__INSERT_SUBMISSION_FROM_CMNT, SQL__INSERT_SUBMISSION_FROM_CMNT_INDX);
	}
	
	if (SQL__INSERT_INTO_USER2SUBCNT_INDX != strlen_constexpr(SQL__INSERT_INTO_USER2SUBCNT_STR)){
		--SQL__INSERT_INTO_USER2SUBCNT_INDX; // Overwrite trailing comma
		constexpr const char* b = " ON DUPLICATE KEY UPDATE count = count + 1";
		memcpy(SQL__INSERT_INTO_USER2SUBCNT + SQL__INSERT_INTO_USER2SUBCNT_INDX,  b,  strlen_constexpr(b));
		SQL__INSERT_INTO_USER2SUBCNT_INDX += strlen_constexpr(b);
		exec_buffer(SQL__INSERT_INTO_USER2SUBCNT, SQL__INSERT_INTO_USER2SUBCNT_INDX);
	}
	
	if (SQL__INSERT_INTO_SUBREDDIT_INDX != strlen_constexpr(SQL__INSERT_INTO_SUBREDDIT_STR)){
		SQL__INSERT_INTO_SUBREDDIT[--SQL__INSERT_INTO_SUBREDDIT_INDX] = 0;
		exec_buffer(SQL__INSERT_INTO_SUBREDDIT, SQL__INSERT_INTO_SUBREDDIT_INDX);
	}
	
	return str2id(replies["data"]["children"][0]["data"]["id"].GetString());
}

void process_all_comments_live(){
	uint64_t last_processed_cmnt_id = 0;
	
	while (true){
		sleep(myrcu::REDDIT_REQUEST_DELAY);
		
		
		mycu::request("https://oauth.reddit.com/r/all/comments/?limit=100&raw_json=1");
		
		rapidjson::Document d;
		
		if (myrcu::try_again(d))
			continue;
		
		last_processed_cmnt_id = process_live_replies(d, last_processed_cmnt_id);
	}
}

int main(){
	compsky::mysql::init_auth(_mysql::buf, _mysql::buf_sz, _mysql::auth, getenv("RSCRAPER_MYSQL_CFG"));
	compsky::mysql::login_from_auth(_mysql::obj, _mysql::auth);
	filter_comment_body::init();
	mycu::init();         // Init CURL
	myrcu::init(getenv("RSCRAPER_REDDIT_CFG")); // Init OAuth
	
	filter_user::init();
	filter_subreddit::init();
	
	process_all_comments_live();
	
	compsky::mysql::wipe_auth(_mysql::buf, _mysql::buf_sz);
}
