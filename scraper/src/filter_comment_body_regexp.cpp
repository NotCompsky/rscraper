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

#include "init_regexp_from_file.hpp"
#include "error_codes.hpp" // for myerr:*
#include "filter_comment_body.hpp"

#include <compsky/regex/named_groups.hpp>
#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <stdio.h> // for fprintf


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace filter_comment_body {


std::vector<char*> reason_name2id;
std::vector<int> groupindx2reason;
std::vector<bool> record_contents;

std::vector<std::vector<uint64_t>> SUBREDDIT_WHITELISTS;
std::vector<std::vector<uint64_t>> SUBREDDIT_BLACKLISTS;


void populate_reason2name(){
	compsky::mysql::query_buffer(&RES1, "SELECT r.id, r.name, IFNULL(w.subreddit,0), IFNULL(b.subreddit,0) FROM reason_matched r LEFT JOIN reason_subreddit_blacklist b ON r.id=b.reason LEFT JOIN reason_subreddit_whitelist w ON r.id=w.reason ORDER BY r.id DESC");
	int reason_id;
	char* name;
	uint64_t subreddit_wl, subreddit_bl;
	constexpr static const compsky::mysql::flag::SizeOfAssigned f;
	size_t name_sz;
	if(compsky::mysql::assign_next_row(RES1, &ROW1, &reason_id, f, &name_sz, &name, &subreddit_wl, &subreddit_bl)){
		// Initialise the vectors
		reason_name2id.reserve(reason_id);
		SUBREDDIT_WHITELISTS.reserve(reason_id);
		SUBREDDIT_BLACKLISTS.reserve(reason_id);
		for (int i = 0;  i < reason_id + 1;  ++i){
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
	} while(compsky::mysql::assign_next_row(RES1, &ROW1, &reason_id, f, &name_sz, &name, &subreddit_wl, &subreddit_bl));
}

void init(){
	populate_reason2name();
	
	filter_comment_body::init_regexp_from_file(reason_name2id, groupindx2reason, record_contents);
	
	constexpr static const compsky::asciify::flag::ChangeBuffer chbuf;
	constexpr static const compsky::asciify::flag::Escape esc;
	
	compsky::asciify::asciify(chbuf, compsky::asciify::BUF, "INSERT IGNORE INTO reason_matched (id,name) VALUES");
	
	for (size_t i = 0;  i < reason_name2id.size();  ++i){
		if (reason_name2id[i] == nullptr)
			// The entry was deleted from the reason_matched table, leaving a discontinuity in the id field
			continue;
		compsky::asciify::asciify("(", i, ",\"", esc, '"', reason_name2id[i], "\"),");
	}
	compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::get_index() - 1); // Ignore trailing comma
	
	compsky::mysql::query_buffer(&RES1,  "SELECT id, subreddit, reason, body FROM regex_test__cmnt_body");
	unsigned int id;
	uint64_t subreddit;
	unsigned int expected_reason;
	char* body;
	unsigned int n_failures = 0;
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &id, &subreddit, &expected_reason, &body)){
		struct cmnt_meta metadata = {
			"author_name IGNORED",
			"subreddit_name IGNORED",
			0, // author field, ignored
			subreddit,
		};
		
		bool ignored_as_yet;
		unsigned int actual_reason;
		actual_reason = match(metadata, body, strlen(body), ignored_as_yet);
		if(actual_reason != expected_reason){
			fprintf(stderr, "cmnt_body regex test #%u failed\n\tExpected:\n\t\tGroup #%u (%s)\n\tActual:\n\t\tGroup #%u (%s)\n\t%s\n", id, expected_reason, reason_name2id[expected_reason], actual_reason, reason_name2id[actual_reason], body);
			++n_failures;
		}
	}
	if (n_failures != 0){
		for (size_t i = 1;  i < groupindx2reason.size();  ++i){
			// Ignore first index - it is the entire match, not a regex group.
			printf("groupindx2reason[%d]\t%d\n\tCorresponding to group\t%s\n", i, groupindx2reason[i], (groupindx2reason[i] < reason_name2id.size()) ? reason_name2id[groupindx2reason[i]] : "(index beyond reason_name2id)");
		}
		// Somewhat duplicated code from init_regexp_from_file.cpp // Possible TODO: Deduplicate
		compsky::mysql::query_buffer(&RES1,  "SELECT data FROM longstrings WHERE name='cmnt_body_regex'");
		char* regexpr_str;
		while(compsky::mysql::assign_next_row(RES1, &ROW1, &regexpr_str)){
			compsky::regex::convert_named_groups(regexpr_str,  regexpr_str,  reason_name2id,  groupindx2reason,  record_contents);
			printf("Using regexp:\n\t%s\n", regexpr_str);
		}
		exit(myerr::TEST_FAILED__REGEX_CMNT_BODY);
	}
}


}
#endif
