/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "rscraper/tagger.hpp"

#include <compsky/asciify/init.hpp>
#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <cstring> // for memcpy, strlen
#include <stdlib.h> // for abort

#ifndef DEBUG
# define printf(...)
#else
# include <stdio.h> // for printf // TMP
#endif


MYSQL_RES* RES;
MYSQL_ROW ROW;

extern "C" char* DST = NULL; // alias for BUF

namespace compsky {
	namespace asciify {
		char* BUF;
		char* ITR;
		constexpr const size_t BUF_SZ = 4096 * 1024;
	}
}

namespace _f {
	constexpr static const compsky::asciify::flag::concat::Start cc_start;
	constexpr static const compsky::asciify::flag::concat::End   cc_end;
	constexpr static const compsky::asciify::flag::Escape esc;
}


constexpr static const char* id_t2_ = "id-t2_";


namespace http_err {
	// TODO: Sort of the const-ness of DST. This should be const char* const
	constexpr static char* const request_too_long = "414";
	constexpr static char* const bad_request = "400";
}


constexpr size_t strlen_constexpr(const char* s){
	// GCC strlen is constexpr; this is apparently a bug
	return (*s)  ?  1 + strlen_constexpr(s + 1)  :  0;
}

bool is_number(const char* s){
	while(*s != 0){
		if (*s < '0'  ||  *s > '9')
			return false;
		++s;
	}
	return true;
}





void id2str(uint64_t id_orig,  char* buf){
	size_t n_digits = 0;
	uint64_t id = id_orig;
	while (id != 0){
		++n_digits;
		id /= 36;
	}
	buf[n_digits] = 0;
	while (id_orig != 0){ // Note that a subreddit id should never be 0
		char digit = id_orig % 36;
		buf[--n_digits] = digit + ((digit<10) ? '0' : 'a' - 10);
		id_orig /= 36;
	}
}

constexpr uint64_t str2id(const char* start,  const char* end){
	// End points to the comma after the id
	uint64_t n = 0;
	for (const char* str = start;  str != end;  ++str){
		n *= (10 + 26);
		if (*str >= '0'  &&  *str <= '9')
			n += *str - '0';
		else
			n += *str - 'a' + 10;
	}
	return n;
}

constexpr
bool is_length_greater_than(const char* str,  const size_t n){
	for(auto i = 0;  i < n;  ++i){
		if (*str == 0)
			return false;
		++str;
	}
	return true;
}

constexpr
bool is_valid_username(const char* str){
	for(auto i = 0;  i < 129;  ++i){
		switch(*str){
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
			
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
			
			case '_':
			case '-':
				// Yes I know a-z and A-Z are guaranteed to be contiguous
				break;
			
			case 0:  return true;
			
			default: return false;
		};
		++str;
	}
	return false;
}

//static_assert(str2id("6l4z3", 0, 5) == 11063919); // /u/AutoModerator
// Causes error in MXE GCC

//static_assert(n_required_bytes("id-t2_foo,id-t2_bar") == strlen_constexpr("{\"foo\":\"#123456\",\"bar\":\"#123456\"}") + 1);

#ifdef CACHE_VALID_IDS
// NOTE: Such caching will likely use up a few megabytes.
uint64_t* reasons;
uint64_t* users;
uint64_t* subreddits;
size_t n_reasons;
size_t n_reasons_log2;
size_t n_users;
size_t n_users_log2;
size_t n_subreddits;
size_t n_subreddits_log2;

size_t log2(size_t m){
	size_t n = 0;
	while (m >>= 1)
		++n;
	return n;
}

/*
IDs can be obtained client-side, probably using Pushshift.
For instance:
	Subreddit ID:
		curl -g 'https://dev.pushshift.io/rr/_search/?source_content_type=application/json&source={%22query%22:{%22match%22:{%22_id%22:6}}}' | jq '.hits.hits[0]._source.display_name'
*/

bool is_cached(uint64_t* ids,  const size_t ids_len,  const size_t ids_len_log2  const uint64_t id){
	// Inspired by Matt Pulver's 2011 article: http://eigenjoy.com/2011/01/21/worlds-fastest-binary-search/
	size_t i = 0;
	for(size_t b = 1 << ids_len_log2;   b != 0;   b >>= 1){
		// TODO: Check for endianness?
		size_t j = i | b;
		if (j >= n_reasons)
			;
		else if (ids[j] <= id)
			i = j;
	}
	return (ids[i] == id);
}
#endif

extern "C"
void init(){
	compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));
	if (compsky::asciify::alloc(compsky::asciify::BUF_SZ))
		abort();
	
#ifdef CACHE_VALID_IDS
	compsky::mysql::query_buffer(
		&RES,
		"SELECT "
			"(SELECT COUNT(*) FROM reason_matched),"
			"(SELECT COUNT(*) FROM user),"
			"(SELECT COUNT(*) FROM subreddit)"
	);
	
	while(compsky::mysql::assign_next_row(RES, &ROW, &n_reasons, &n_users, &n_subreddits));
	
	n_reasons_log2 = log2(n_reasons);
	n_users_log2 = log2(n_users);
	n_subreddits_log2 = log2(n_subreddits);
	
	uint64_t* buf = (uint64_t*)malloc((n_reasons + n_users + n_subreddits) * sizeof(uint64_t));
	// Prefer large single mallocs to multiple smaller mallocs
	// Both to reduce system calls, and also reduce memory fragmentation
	
	if (unlikely(buf == nullptr))
		abort();
	
	reasons    = buf;
	users      = reasons + n_reasons;
	subreddits = users + n_users;
	
	compsky::mysql::query(
		&RES,
		"SELECT "
			"(SELECT id FROM reason_matched LIMIT ", n_reasons, "),"
			"(SELECT id FROM user LIMIT ", n_users, "),"
			"(SELECT id FROM subreddit LIMIT ", n_subreddits, "),"
	);
	// NOTE: MySQL implicitly orders (ascending) by ID, as it is the primary key.
	uint64_t n;
	while(compsky::mysql::assign_next_row(RES, &ROW, &n))
		*(buf++) = n;
#endif
}

extern "C"
void exit_mysql(){
	compsky::mysql::exit_mysql();
}

size_t generate_user_id_list_string(const char* csv){
	char* const buf_init = compsky::asciify::ITR;
	bool current_id_valid = (*csv == '_');
	printf("%c\n", *csv);
	++csv; // Skip last character of first prefix (no need to check for 0 - done in switch)
	const char* current_id_start = csv;
	while (true){
		switch(*csv){
			case 0:
			case ',':
				if (current_id_valid){
					const uint64_t id = str2id(current_id_start, csv);
					
					compsky::asciify::asciify(id);
					compsky::asciify::asciify(',');
				}
				for (auto i = 0;  i < 6;  ++i)
					// Safely skip the prefix ("id-t2_")
					if (*(++csv) == 0)
						// Good input would end only on k = 0 (corresponding to 'case 0')
						// Otherwise, there was not the expected prefix in after the comma
						return (uintptr_t)compsky::asciify::ITR - (uintptr_t)buf_init;
				current_id_valid = (*csv == '_');
				printf("%c\n", *csv);
				current_id_start = csv + 1; // Start at character after comma
				break;
		}
		++csv;
	}
}

/*
extern "C"
void generate_id_list(const char* tblname,  const char** names,  uint64_t* ls){
	// NOTE: Maximum length of ls is known when this function is called (it is equal to length of names)
	compsky::mysql::query(&RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", names, _f::cc_end,  "')");
	uint64_t id;
	while(compsky::mysql::assign_next_row(RES, &ROW, &id))
		*(ls++) = id;
}

extern "C"
uint64_t* generate_id_list_string(const char* tblname,  const char** names){
	size_t max_n_IDs = 1; // For trailing null
	for (const char** itr = names;  itr != nullptr;  ++itr)
		++max_n_IDs;
	void* dummy = malloc(sizeof(uint64_t) * max_n_IDs);
	if (dummy == nullptr)
		exit(1036);
	uint64_t* const ls_start = (uint64_t*)dummy;
	uint64_t* ls = ls_start;
	compsky::mysql::query(&RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", names, _f::cc_end,  "')");
	uint64_t id;
	while(compsky::mysql::assign_next_row(RES, &ROW, &id))
		*(ls++) = id;
	*ls = 0;
	return ls_start;
}
*/

extern "C"
const char* generate_id_list_string(const char* tblname,  const char** names){
	size_t max_n_IDs = 1; // For trailing null
	for (const char** itr = names;  itr != nullptr;  ++itr)
		++max_n_IDs;
	void* const dummy = malloc(strlen_constexpr(" WHERE id IN (")  +  20 * max_n_IDs  +  2); // uint64_t has 19 digits maximum; add 1 for comma. Trailing close bracket and null byte.
	if (dummy == nullptr)
		exit(1036);
	char* const start = (char* const)dummy;
	char* itr = start;
	compsky::mysql::query(&RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", 3, names, max_n_IDs, _f::cc_end,  "')");
	char* id;
	while(compsky::mysql::assign_next_row(RES, &ROW, &id)){
		memcpy(itr,  id,  strlen(id));
		itr += strlen(id);
		*(itr++) = ',';
	}
	*(itr++) = ')';
	*itr = 0;
	return start;
}

extern "C"
void csv2cls(const char* csv,  const char* tagcondition,  const char* reasoncondition){
	/*
	Both 'tagcondition' and 'reasoncondition' are arrays of IDs used to filter the objects that populate the users' tag flair.
	E.g. if tagcondition==nullptr, all tags are ignored.
	If tagcondition=="", all tags are used.
	If tagcondition=="AND id=1", only the tag with ID of 1 is used when generating the flair.
	There is no effort to account for the case where both tags and reasons are null. There is no use case for such a configuration.
	*/
	/*
	The input id-t2_IDSTR,id-t2_IDSTR2,id-t2_IDSTR3, ... maps to {"IDSTR":"#0123456", ... }
	
	id_t2_ cancels out 0123456 on all values seperated by commas
	
	So IDSTR,IDSTR2,IDSTR3 ... maps to {"IDSTR":"#","IDSTR2":"#", ... }
	
	So ,, ... maps to {"":"#","":"#", ... }
	
	So strlen(output)  =  2 + n_commas(csv)*6 + strlen(output)
	*/
	
	// Convert id-t2_ABCDEF,id-t2_abcdefg,id-t2_12345  to  colour hex codes
	// The former is longer than the latter, so can reuse the same string as the final output
	// SQL statement might still be longer though, so have to create new string for it
	
	for (auto i = 0;  i < 5;  ++i){
		// Safely skip first prefix bar the last character
		if (*(csv++) == 0){
			compsky::asciify::ITR = compsky::asciify::BUF + 1;
			goto goto_results;
		}
	}
	
	compsky::asciify::reset_index();
	compsky::asciify::asciify("SELECT * FROM (");

	constexpr static const char* stmt_t_1 = 
		"SELECT A.user_id, SUM(A.c), SUM(A.c), SUM(A.r*A.c), SUM(A.g*A.c), SUM(A.b*A.c), SUM(A.a*A.c), A.tag_id "
		"FROM tag2category t2c "
		"JOIN ( "
			"SELECT u2scc.user_id, s2t.tag_id, SUM(u2scc.count) AS c, t.r, t.g, t.b, t.a "
			"FROM user2subreddit_cmnt_count u2scc, subreddit s, subreddit2tag s2t, tag t "
			"WHERE s.id=u2scc.subreddit_id AND s.id=s2t.subreddit_id AND t.id=s2t.tag_id AND u2scc.user_id IN (";

	constexpr static const char* stmt_t_2 =
			// ")" // Closing bracket added seperately 
			" GROUP BY u2scc.user_id, s2t.tag_id, t.r, t.g, t.b, t.a"
		") A ON t2c.tag_id = A.tag_id "
		"GROUP BY A.user_id, t2c.category_id";

	constexpr static const char* stmt_m_1 =
		"SELECT c.author_id AS user_id, COUNT(m.id), 1, m.r, m.g, m.b, m.a, m.id "
		"FROM comment c, reason_matched m "
		"WHERE c.reason_matched=m.id AND c.author_id IN (";

	constexpr static const char* stmt_m_2 =
		/* ) */ " GROUP BY c.author_id, m.name, m.r, m.g, m.b, m.a"; // First ')' is not necessary as it is already copied by 'n_bytes_of_IDs' - because the last trailing comma is recorded by n_bytes_of_IDs as the comma is not stripped before then. This is slightly undesirable only for readability, but the alternative is to decrement BUF_INDX within the generate_user_id_list_string function, which would greatly complicate it.

	if (tagcondition != nullptr){
		memcpy(compsky::asciify::ITR,  stmt_t_1,  strlen_constexpr(stmt_t_1));
		compsky::asciify::ITR += strlen_constexpr(stmt_t_1);
		char* const start_of_user_IDs = compsky::asciify::ITR;
		const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
		if (n_bytes_of_IDs == 0){
			// No valid IDs were found
			DST = "{}";
			return;
		}
		--compsky::asciify::ITR; // Remove trailing comma
		compsky::asciify::asciify(')'); // Close 'WHERE id IN (' condition
		compsky::asciify::asciify(tagcondition); // Could be empty string, or "AND t.id IN (...)", etc.
		memcpy(compsky::asciify::ITR,  stmt_t_2,  strlen_constexpr(stmt_t_2));
		compsky::asciify::ITR += strlen_constexpr(stmt_t_2);
		if (reasoncondition != nullptr){
			compsky::asciify::asciify(" UNION ALL SELECT 0, 0, 0, 0, 0, 0, 0, 0 UNION ALL ");
			memcpy(compsky::asciify::ITR,  stmt_m_1,  strlen_constexpr(stmt_m_1));
			compsky::asciify::ITR += strlen_constexpr(stmt_m_1);
			memcpy(compsky::asciify::ITR,  start_of_user_IDs,  n_bytes_of_IDs);
			compsky::asciify::ITR += n_bytes_of_IDs;
			// No need to add closing bracket - copied by the above
			compsky::asciify::asciify(reasoncondition); // Could be empty string, or "AND t.id IN (...)", etc.
			memcpy(compsky::asciify::ITR,  stmt_m_2,  strlen_constexpr(stmt_m_2));
			compsky::asciify::ITR += strlen_constexpr(stmt_m_2);
		}
	} else { // Realistically, this should be 'reasons != nullptr' - though no effort is made to check that this holds
		memcpy(compsky::asciify::ITR,  stmt_m_1,  strlen_constexpr(stmt_m_1));
		compsky::asciify::ITR += strlen_constexpr(stmt_m_1);
		const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
		if (n_bytes_of_IDs == 0){
			// No valid IDs were found
			DST = "{}";
			return;
		}
		--compsky::asciify::ITR; // Remove trailing comma
		compsky::asciify::asciify(')');
		compsky::asciify::asciify(reasoncondition); // Could be empty string, or "AND t.id IN (...)", etc.
		memcpy(compsky::asciify::ITR,  stmt_m_2,  strlen_constexpr(stmt_m_2));
		compsky::asciify::ITR += strlen_constexpr(stmt_m_2);
	}
	compsky::asciify::asciify(") A ORDER BY user_id");
	
	printf("QRY: %.*s\n",  compsky::asciify::get_index(),  compsky::asciify::BUF); // TMP
	
	compsky::mysql::query_buffer(&RES, compsky::asciify::BUF, compsky::asciify::get_index());
	
	
	compsky::asciify::reset_index();
	
	//[ We obtain an (erroneous) prefix of "]," in the following loop
	// These two characters are later overwritten with "[{"
	
	{
	uint64_t last_id = 0;
	uint64_t id;
	uint64_t n_cmnts;
	uint64_t div_rgb_by;
	double r, g, b, a;
	char* tag_or_reason_id;
	char id_str[19 + 1];
	size_t id_str_len;
	constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
	while (compsky::mysql::assign_next_row(RES, &ROW, &id, &n_cmnts, &div_rgb_by, &r, &g, &b, &a, &tag_or_reason_id)){
		if (id == 0){
			// i.e. we are in between the real selects in the union
			compsky::asciify::asciify("]},{[");
		}
		
		const size_t max_new_entry_size = strlen_constexpr("],\"id-t2_abcdefghijklm\":[[\"rgba(255,255,255,1.000)\",\"01234567890123456789 01234567890123456789\"],");
		
		if (compsky::asciify::get_index() + max_new_entry_size + 1 > compsky::asciify::BUF_SZ )
			//{ +1 is to account for the terminating '}' char.
			break;
		
		if (id != last_id){
			--compsky::asciify::ITR;  // Overwrite trailing comma left by RGBs
			id2str(id, id_str);
			compsky::asciify::asciify("],\"id-t2_",  id_str,  "\":[");
			last_id = id;
		}
		
		compsky::asciify::asciify(
			"[\"rgba(",
			+(uint8_t)(255.0 * r / (double)div_rgb_by),  ',',
			+(uint8_t)(255.0 * g / (double)div_rgb_by),  ',',
			+(uint8_t)(255.0 * b / (double)div_rgb_by),  ',',
			f, (double)(a / (double)div_rgb_by), 3,
			"),",
			tag_or_reason_id,
			',',
			n_cmnts,
			"],"
		);
	}
	}
	goto_results:
	if (compsky::asciify::get_index() == 5){
		DST = "[{},{}]";
	} else {
		DST = compsky::asciify::BUF;
		
		const bool first_results_nonempty = (compsky::asciify::BUF[1] == ',');
		const bool secnd_results_nonempty = (*(compsky::asciify::ITR-1) == ',');
		if (!first_results_nonempty){
			// Only first results set is empty
			DST += 1;
			DST[0] = '[';
			DST[1] = '{';
			DST[2] = '}';
			DST[3] = ',';
			--compsky::asciify::ITR; // Overwrite trailing comma
			compsky::asciify::asciify(']', '}', ']');
		} else {
			DST[0] = '[';
			DST[1] = '{';
			
			if (secnd_results_nonempty) {
				// Neither is empty
				--compsky::asciify::ITR; // Overwrite trailing comma
				compsky::asciify::asciify(']', '}', ']');
			} else {
				// Only second is empty
				--compsky::asciify::ITR; // Overwrite trailing opening square bracket
				compsky::asciify::asciify('}', ']');
			}
		}
		
		compsky::asciify::asciify('\0');
	}
}


extern "C"
void comments_given_username(const char* const reasonfilter,  const char* const name){
	/*
	TODO: 	Move from name-based searches to ID-based.
			This would allow a very quick cache check, avoiding expensive resultless MySQL queries.
	
	if (!is_cached(users, n_users, n_users_log2, name)){
		DSDT = http_err::not_in_database;
		return;
	}
	*/
	
	if (unlikely(!is_valid_username(name))){
		/*
		This is the length of the field in the user table
		Exceeding this length is likely malicious.
		One would have to push a name of length 4MiB in order to cause a buffer overflow - hence why it is far more performant to simply check that the length is under 128, rather than calling strlen on what may be incredibly long.
		*/
		DST = http_err::bad_request;
		return;
	}
	
	compsky::mysql::query(
		&RES,
		"SELECT m.name, r.name, s.id, c.id, c.created_at "
		"FROM comment c, subreddit r, submission s, user u, reason_matched m "
		"WHERE u.name=\"", name, "\" "
		  "AND c.author_id=u.id "
		  "AND s.id=c.submission_id "
		  "AND r.id=s.subreddit_id "
		  "AND m.id=c.reason_matched ",
		  reasonfilter,
		  " ORDER BY c.created_at DESC "
		  "LIMIT 1000"
	);
	char* reason;
	char* subreddit_name;
	uint64_t submission_id;
	uint64_t comment_id;
	char submission_id_str[19 + 1];
	char comment_id_str[19 + 1];
	char* created_at;
	compsky::asciify::reset_index();
	compsky::asciify::asciify('[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &reason, &subreddit_name, &submission_id, &comment_id, &created_at)){
		id2str(submission_id, submission_id_str);
		id2str(comment_id,    comment_id_str);
		compsky::asciify::asciify(
			'[',
				'"', _f::esc, '"', reason, '"', ',',
				'"', _f::esc, '"', subreddit_name, '"', ',',
				created_at, ',',
				'"', submission_id_str, "/_/", comment_id_str, '"',
			']',
			','
		);
	}
	if(compsky::asciify::get_index() > 1)
		--compsky::asciify::ITR;
	compsky::asciify::asciify(']', '\0');
	DST = compsky::asciify::BUF;
}

extern "C"
void comments_given_reason(const char* const reasonfilter,  const char* const reason_id){
	if (unlikely(!is_number(reason_id))){
		DST = http_err::bad_request;
		return;
	}
	
	compsky::mysql::query(
		&RES,
		"SELECT r.name, s.id, c.id, c.created_at "
		"FROM comment c, subreddit r, submission s, reason_matched m "
		"WHERE m.id=", reason_id, " "
		  "AND s.id=c.submission_id "
		  "AND r.id=s.subreddit_id "
		  "AND m.id=c.reason_matched ",
		  reasonfilter,
		  " ORDER BY c.created_at DESC "
		  "LIMIT 100"
	);
	char* subreddit_name;
	uint64_t submission_id;
	uint64_t comment_id;
	char submission_id_str[19 + 1];
	char comment_id_str[19 + 1];
	char* created_at;
	compsky::asciify::reset_index();
	compsky::asciify::asciify('[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_name, &submission_id, &comment_id, &created_at)){
		id2str(submission_id, submission_id_str);
		id2str(comment_id,    comment_id_str);
		compsky::asciify::asciify(
			'[',
				'"', _f::esc, '"', subreddit_name, '"', ',',
				created_at, ',',
				'"', submission_id_str, "/_/", comment_id_str, '"',
			']',
			','
		);
	}
	if(compsky::asciify::get_index() > 1)
		--compsky::asciify::ITR;
	compsky::asciify::asciify(']', '\0');
	DST = compsky::asciify::BUF;
}

extern "C"
void subreddits_given_reason(const char* const reasonfilter,  const char* const reason_id){
	if (unlikely(!is_number(reason_id))){
		DST = http_err::bad_request;
		return;
	}
	
	compsky::mysql::query(
		&RES,
		"SELECT r.name, COUNT(c.id)/s2cc.count AS count "
		"FROM subreddit r, submission s, comment c, reason_matched m, subreddit2cmnt_count s2cc "
		"WHERE m.id=", reason_id, " "
		  "AND r.id=s.subreddit_id "
		  "AND s.id=c.submission_id "
		  "AND c.reason_matched=m.id ",
		  "AND s2cc.id=r.id "
		  "AND s2cc.count>1000 ",
		  reasonfilter,
		"GROUP BY r.name "
		"ORDER BY count DESC "
		"LIMIT 100"
	);
	char* subreddit_name;
	char* proportion;
	compsky::asciify::reset_index();
	compsky::asciify::asciify('[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_name, &proportion)){
		compsky::asciify::asciify(
			'[',
				'"', _f::esc, '"', subreddit_name, '"', ',',
				proportion,
			']',
			','
		);
	}
	if(compsky::asciify::get_index() > 1)
		--compsky::asciify::ITR;
	compsky::asciify::asciify(']', '\0');
	DST = compsky::asciify::BUF;
}


extern "C"
void get_all_reasons(const char* const reasonfilter){
	compsky::mysql::query(
		&RES,
		"SELECT m.name, m.id "
		"FROM reason_matched m "
		"WHERE TRUE ",
		  reasonfilter
	);
	char* name;
	char* reason_id;
	compsky::asciify::reset_index();
	compsky::asciify::asciify('{');
	while(compsky::mysql::assign_next_row(RES, &ROW, &name, &reason_id)){
		compsky::asciify::asciify(
			"\"", reason_id, "\":"
				"\"", _f::esc, '"', name, "\""
			","
		);
	}
	if(compsky::asciify::get_index() > 1)
		--compsky::asciify::ITR;
	compsky::asciify::asciify('}', '\0');
	DST = compsky::asciify::BUF;
}

extern "C"
void get_all_tags(const char* const tagfilter){
	// TODO: Reduce code duplication (virtually identical to get_all_reasons)
	compsky::mysql::query(
		&RES,
		"SELECT t.name, t.id "
		"FROM tag t "
		"WHERE TRUE ",
		tagfilter
	);
	char* name;
	char* id;
	compsky::asciify::reset_index();
	compsky::asciify::asciify('{');
	while(compsky::mysql::assign_next_row(RES, &ROW, &name, &id)){
		compsky::asciify::asciify(
			"\"", id, "\":"
				"\"", _f::esc, '"', name, "\""
			","
		);
	}
	if(compsky::asciify::get_index() > 1)
		--compsky::asciify::ITR;
	compsky::asciify::asciify('}', '\0');
	DST = compsky::asciify::BUF;
}
