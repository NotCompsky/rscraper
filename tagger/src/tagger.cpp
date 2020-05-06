/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "rscraper/tagger.hpp"

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

namespace sql {
	MYSQL* obj;
	constexpr const size_t buf_sz = 512;
	char buf[buf_sz];
}

char* BUF;
char* ITR;
constexpr const size_t BUF_SZ = 4096 * 1024;
extern "C" char* DST = NULL; // alias for BUF

namespace _f {
	constexpr static const compsky::asciify::flag::concat::Start cc_start;
	constexpr static const compsky::asciify::flag::concat::End   cc_end;
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::StrLen strlen;
}


constexpr static const char* id_t2_ = "id-t2_";


namespace http_err {
	// TODO: Sort of the const-ness of DST. This should be const char* const
	constexpr static char* const request_too_long = "414";
	constexpr static char* const bad_request = "400";
	constexpr static char* const not_in_database = "400";
}


constexpr size_t strlen_constexpr(const char* s){
	// GCC strlen is constexpr; this is apparently a bug
	return (*s)  ?  1 + strlen_constexpr(s + 1)  :  0;
}


size_t get_index(const char* const itr,  const char* const buf){
	return (uintptr_t)itr - (uintptr_t)buf;
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

constexpr
uint64_t str2id(const char* str){
	// End points to the comma after the id
	uint64_t n = 0;
	while(*str != 0){
		n *= (10 + 26);
		if (*str >= '0'  &&  *str <= '9')
			n += *str - '0';
		else
			n += *str - 'a' + 10;
		++str;
	}
	return n;
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
			case 'a' ... 'z':
			case 'A' ... 'Z':
			case '_':
			case '-':
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

bool is_cached(const uint64_t* ids,  const size_t ids_len,  const size_t ids_len_log2,  const uint64_t id){
	// Inspired by Matt Pulver's 2011 article: http://eigenjoy.com/2011/01/21/worlds-fastest-binary-search/
	size_t i = 0;
	for(size_t b = 1 << ids_len_log2;   b != 0;   b >>= 1){
		// TODO: Check for endianness?
		size_t j = i | b;
		if (j >= ids_len)
			;
		else if (ids[j] <= id)
			i = j;
	}
	return (ids[i] == id);
}

extern "C"
void init(){
	compsky::mysql::init(sql::obj, sql::buf, sql::buf_sz, getenv("RSCRAPER_MYSQL_CFG"));
	BUF = (char*)malloc(BUF_SZ);
	if (BUF == nullptr)
		abort();
	
	compsky::mysql::query_buffer(
		sql::obj,
		RES,
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
		sql::obj,
		RES,
		"(SELECT id FROM reason_matched LIMIT ", n_reasons, ")"
		" UNION ALL "
		"(SELECT id FROM user LIMIT ", n_users, ")"
		" UNION ALL "
		"(SELECT id FROM subreddit LIMIT ", n_subreddits, ")"
	);
	// NOTE: MySQL implicitly orders (ascending) by ID, as it is the primary key.
	uint64_t n;
	while(compsky::mysql::assign_next_row(RES, &ROW, &n))
		*(buf++) = n;
}

extern "C"
void exit_mysql(){
	compsky::mysql::wipe_auth(sql::buf, sql::buf_sz);
}

size_t generate_user_id_list_string(const char* csv){
	char* const buf_init = ITR;
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
					
					compsky::asciify::asciify(ITR, id);
					compsky::asciify::asciify(ITR, ',');
				}
				for (auto i = 0;  i < 6;  ++i)
					// Safely skip the prefix ("id-t2_")
					if (*(++csv) == 0)
						// Good input would end only on k = 0 (corresponding to 'case 0')
						// Otherwise, there was not the expected prefix in after the comma
						return get_index(ITR, buf_init);
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
	compsky::mysql::query(sql::obj, RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", names, _f::cc_end,  "')");
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
	compsky::mysql::query(sql::obj, RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", names, _f::cc_end,  "')");
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
	compsky::mysql::query(sql::obj, RES,  "SELECT id FROM ",  tblname,  " WHERE name IN ('",  _f::cc_start, "','", 3, names, max_n_IDs, _f::cc_end,  "')");
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
		if (unlikely(*(csv++) == 0)){
			ITR = BUF + 5; // Just to trigger 'if get_index() == 5'
			goto goto_results;
		}
	}

	constexpr static const char* stmt_t_1 = 
		"SELECT A.user_id, SUM(A.c), SUM(A.c) AS distinctname"
			", SUM(A.r*A.c), SUM(A.g*A.c), SUM(A.b*A.c), SUM(A.a*A.c), A.tag_id "
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

	ITR = BUF;
	
	if (tagcondition != nullptr){
		compsky::asciify::asciify(ITR, _f::strlen, stmt_t_1, strlen_constexpr(stmt_t_1));
		char* const start_of_user_IDs = ITR;
		const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
		if (n_bytes_of_IDs == 0){
			// No valid IDs were found
			DST = "{}";
			return;
		}
		--ITR; // Remove trailing comma
		compsky::asciify::asciify(ITR, ')'); // Close 'WHERE id IN (' condition
		compsky::asciify::asciify(ITR, tagcondition); // Could be empty string, or "AND t.id IN (...)", etc.
		compsky::asciify::asciify(ITR, _f::strlen, stmt_t_2, strlen_constexpr(stmt_t_2));
		if (reasoncondition != nullptr){
			compsky::asciify::asciify(ITR, " UNION ALL SELECT 0, 0, 0, 0, 0, 0, 0, 0 UNION ALL ");
			compsky::asciify::asciify(ITR, _f::strlen, stmt_m_1, strlen_constexpr(stmt_m_1));
			compsky::asciify::asciify(ITR, _f::strlen, start_of_user_IDs, n_bytes_of_IDs);
			// No need to add closing bracket - copied by the above
			compsky::asciify::asciify(ITR, reasoncondition); // Could be empty string, or "AND t.id IN (...)", etc.
			compsky::asciify::asciify(ITR, _f::strlen, stmt_m_2, strlen_constexpr(stmt_m_2));
		}
	} else { // Realistically, this should be 'reasons != nullptr' - though no effort is made to check that this holds
		compsky::asciify::asciify(ITR, _f::strlen, stmt_m_1, strlen_constexpr(stmt_m_1));
		const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
		if (n_bytes_of_IDs == 0){
			// No valid IDs were found
			DST = "{}";
			return;
		}
		--ITR; // Remove trailing comma
		compsky::asciify::asciify(ITR, ')');
		compsky::asciify::asciify(ITR, reasoncondition); // Could be empty string, or "AND t.id IN (...)", etc.
		compsky::asciify::asciify(ITR, _f::strlen, stmt_m_2, strlen_constexpr(stmt_m_2));
	}
	
	printf("QRY: %.*s\n",  get_index(ITR, BUF),  BUF); // TMP
	
	compsky::mysql::query_buffer(sql::obj, RES, BUF, get_index(ITR, BUF));
	
	
	ITR = BUF;
	++ITR;
	
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
	char* position_to_overwrite_with_open_square_brkt = nullptr;
	constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
	while (compsky::mysql::assign_next_row(RES, &ROW, &id, &n_cmnts, &div_rgb_by, &r, &g, &b, &a, &tag_or_reason_id)){
		if (id == 0){
			// i.e. we are in between the real selects in the union
			compsky::asciify::asciify(ITR, "]}"); //[ ",{" is added afterwards, overwriting the "]," that is otherwise placed in this position}
			position_to_overwrite_with_open_square_brkt = ITR;
			last_id = 0;
			continue;
		}
		
		const size_t max_new_entry_size = strlen_constexpr("],\"id-t2_abcdefghijklm\":[[\"rgba(255,255,255,1.000)\",\"01234567890123456789 01234567890123456789\"],");
		
		if (get_index(ITR, BUF) + max_new_entry_size + 1 > BUF_SZ){
			// Give up; we've provided enough results by now!
			//{ +1 is to account for the terminating '}' char.
			mysql_free_result(RES);
			break;
		}
		
		if (id == last_id){
			compsky::asciify::asciify(ITR, ',');
		} else {
			id2str(id, id_str);
			compsky::asciify::asciify(ITR, "],\"id-t2_",  id_str,  "\":[");
			// The previous line leads to the assertion that first_results_nonempty==(compsky::asciify::BUF[1]==',')
			last_id = id;
		}
		
		compsky::asciify::asciify(
			ITR,
			"[\"",
			+(uint8_t)(255.0 * r / (double)div_rgb_by),  ',',
			+(uint8_t)(255.0 * g / (double)div_rgb_by),  ',',
			+(uint8_t)(255.0 * b / (double)div_rgb_by),  ',',
			f, (double)(a / (double)div_rgb_by), 3,
			"\",",
			tag_or_reason_id,
			',',
			n_cmnts,
			"]" // Leads to the assertion that secnd_results_nonempty==(*(compsky::asciify::ITR-1)==']')
		);
	}
	if (position_to_overwrite_with_open_square_brkt != nullptr){
		position_to_overwrite_with_open_square_brkt[0] = ',';
		position_to_overwrite_with_open_square_brkt[1] = '{';
	}
	}
	goto_results:
	// TODO: Account for cases ((tagcondition==nullptr), (reasoncondition==nullptr))
	if (get_index(ITR, BUF) == 5){
		DST = "[{},{}]";
	} else {
		DST = BUF + 1;
		
		const bool first_results_nonempty = (DST[1] == ',');   // Begins with ],"id-t2_
		const bool secnd_results_nonempty = (*(ITR-1) == ']'); // Ends   with ]
		if (!first_results_nonempty){
			// Only first results set is empty
			--DST;
			DST[0] = '[';
			DST[1] = '{';
			DST[2] = '}';
			DST[3] = ',';
			DST[4] = '{';
			compsky::asciify::asciify(ITR, ']', '}', ']');
		} else {
			DST[0] = '[';
			DST[1] = '{';
			
			if (secnd_results_nonempty) {
				// Neither is empty
				compsky::asciify::asciify(ITR, ']', '}', ']');
			} else {
				// Only second is empty
				ITR += 2; // Account for position_to_overwrite_with_open_square_brkt
				compsky::asciify::asciify(ITR, '}', ']');
			}
		}
		
		compsky::asciify::asciify(ITR, '\0');
	}
}


template<typename Number1,  typename String,  typename Number2>
void things_given_x__asciify__isi(Number1 a,  String b,  Number2 c){
	compsky::asciify::asciify(
		ITR,
		a, ',',
		'"', _f::esc, '"', b, '"', ',',
		c
	);
}
template<typename String,  typename Number>
void things_given_x__asciify__si(String b,  Number c){
	compsky::asciify::asciify(
		ITR,
		'"', _f::esc, '"', b, '"', ',',
		c
	);
}


void things_given_x__asciify(char* a, char* b, char* c, uint64_t,  const char* const,  float){
	things_given_x__asciify__isi(a,b,c);
}
void things_given_x__asciify(char* a, char* b, char* c, uint64_t,  const char* const,  uint64_t){
	things_given_x__asciify__isi(a,b,c);
}

void things_given_x__asciify(char* b, char* c, const char* const,  uint64_t){
	things_given_x__asciify__si(b,c);
}


template<typename... Args>
void things_given(Args... output){
	ITR = BUF;
	compsky::asciify::asciify(ITR, '[');
	
	constexpr const size_t n_output_args = sizeof...(Args);
	if constexpr(n_output_args == 3){
		char* a;
		char* b;
		char* c;
		while(compsky::mysql::assign_next_row(RES, &ROW, &a, &b, &c)){
			compsky::asciify::asciify(
				ITR,
				'['
			);
			things_given_x__asciify(a, b, c, output...);
			compsky::asciify::asciify(
				ITR,
				']',
				','
			);
		}
	}
	
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, ']', '\0');
	DST = BUF;
}


extern "C"
void subreddits_given_userid(const char* const tagfilter,  const char* const id_str){
	// TODO: De-duplication (comments_given_username)
	
	const uint64_t id = str2id(id_str + 6); // Skip "id-t2_" prefix
	
	if (unlikely(!is_cached(users, n_users, n_users_log2, id))){
		// WARNING: May be annoying if users cache is not updated often.
		DST = http_err::not_in_database;
		return;
	}
	
	// Dummy variables for template
	uint64_t count;
	char* subreddit_name;
	uint64_t tag_id;
	
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT u2scc.count, r.name, s2t.tag_id "
		"FROM user u, user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit r "
		"WHERE u.id=", id, " "
		"AND u2scc.user_id=u.id "
		"AND r.id=u2scc.subreddit_id "
		"AND s2t.subreddit_id=u2scc.subreddit_id "
		"AND t.id=s2t.tag_id ", // for tagfilter - hopefully optimised out if it is just a condition on t.id
		tagfilter,
		"LIMIT 1000"
	);
	things_given(
		count,
		subreddit_name,
		tag_id
	);
}


extern "C"
void comments_given_userid(const char* const reasonfilter,  const char* const id_str){
	// TODO: De-duplication (comments_given_username)
	
	const uint64_t id = str2id(id_str + 6); // Skip "id-t2_" prefix
	
	if (unlikely(!is_cached(users, n_users, n_users_log2, id))){
		// WARNING: May be annoying if users cache is not updated often.
		DST = http_err::not_in_database;
		return;
	}
	
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT m.id, r.name, s.id, c.id, c.created_at "
		"FROM comment c, subreddit r, submission s, user u, reason_matched m "
		"WHERE u.id=", id, " "
		  "AND c.author_id=u.id "
		  "AND s.id=c.submission_id "
		  "AND r.id=s.subreddit_id "
		  "AND m.id=c.reason_matched ",
		  reasonfilter,
		  " ORDER BY c.created_at DESC "
		  "LIMIT 1000"
	);
	char* reason_id;
	char* subreddit_name;
	uint64_t submission_id;
	uint64_t comment_id;
	char submission_id_str[19 + 1];
	char comment_id_str[19 + 1];
	char* created_at;
	ITR = BUF;
	compsky::asciify::asciify(ITR, '[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, &subreddit_name, &submission_id, &comment_id, &created_at)){
		id2str(submission_id, submission_id_str);
		id2str(comment_id,    comment_id_str);
		compsky::asciify::asciify(
			ITR,
			'[',
				reason_id, ',',
				'"', subreddit_name, '"', ',', // No need to escape '"' - it would be an invalid subreddit name
				created_at, ',',
				'"', submission_id_str, "/_/", comment_id_str, '"',
			']',
			','
		);
	}
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, ']', '\0');
	DST = BUF;
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
		sql::obj,
		RES,
		"SELECT m.id, r.name, s.id, c.id, c.created_at "
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
	char* reason_id;
	char* subreddit_name;
	uint64_t submission_id;
	uint64_t comment_id;
	char submission_id_str[19 + 1];
	char comment_id_str[19 + 1];
	char* created_at;
	ITR = BUF;
	compsky::asciify::asciify(ITR, '[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &reason_id, &subreddit_name, &submission_id, &comment_id, &created_at)){
		id2str(submission_id, submission_id_str);
		id2str(comment_id,    comment_id_str);
		compsky::asciify::asciify(
			ITR,
			'[',
				reason_id, ',',
				'"', subreddit_name, ',',
				created_at, ',',
				'"', submission_id_str, "/_/", comment_id_str, '"',
			']',
			','
		);
	}
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, ']', '\0');
	DST = BUF;
}

extern "C"
void comments_given_reason(const char* const reasonfilter,  const char* const reason_id){
	if (unlikely(!is_number(reason_id))){
		DST = http_err::bad_request;
		return;
	}
	
	compsky::mysql::query(
		sql::obj,
		RES,
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
	ITR = BUF;
	compsky::asciify::asciify(ITR, '[');
	while(compsky::mysql::assign_next_row(RES, &ROW, &subreddit_name, &submission_id, &comment_id, &created_at)){
		id2str(submission_id, submission_id_str);
		id2str(comment_id,    comment_id_str);
		compsky::asciify::asciify(
			ITR,
			'[',
				'"', _f::esc, '"', subreddit_name, '"', ',',
				created_at, ',',
				'"', submission_id_str, "/_/", comment_id_str, '"',
			']',
			','
		);
	}
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, ']', '\0');
	DST = BUF;
}

extern "C"
void subreddits_correlation_to_reasons(const char* const reasonfilter){
	// Dummy variables for template
	uint64_t reason_id;
	char* subreddit_name;
	float proportion;
	
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT m.id, r.name, COUNT(c.id)/s2cc.count AS count "
		"FROM subreddit r, submission s, comment c, reason_matched m, subreddit2cmnt_count s2cc "
		"WHERE r.id=s.subreddit_id "
		  "AND s.id=c.submission_id "
		  "AND c.reason_matched=m.id "
		  "AND s2cc.id=r.id "
		  "AND s2cc.count>1000 ",
		  reasonfilter,
		"GROUP BY r.name "
		"HAVING count>10 "
		"ORDER BY count DESC "
		"LIMIT 1000"
	);
	things_given(
		reason_id,
		subreddit_name,
		proportion
	);
}

extern "C"
void subreddits_given_reason(const char* const reasonfilter,  const char* const reason_id){
	if (unlikely(!is_number(reason_id))){
		DST = http_err::bad_request;
		return;
	}
	
	// Dummy variables for template
	char* subreddit_name;
	float proportion;
	
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT r.name, COUNT(c.id)/s2cc.count AS count "
		"FROM subreddit r, submission s, comment c, reason_matched m, subreddit2cmnt_count s2cc "
		"WHERE m.id=", reason_id, " "
		  "AND r.id=s.subreddit_id "
		  "AND s.id=c.submission_id "
		  "AND c.reason_matched=m.id "
		  "AND s2cc.id=r.id "
		  "AND s2cc.count>1000 ",
		  reasonfilter,
		"GROUP BY r.name "
		"HAVING count>10 "
		"ORDER BY count DESC "
		"LIMIT 100"
	);
	things_given(
		subreddit_name,
		proportion
	);
}


extern "C"
void get_all_reasons(const char* const reasonfilter){
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT m.name, m.id "
		"FROM reason_matched m "
		"WHERE TRUE ",
		  reasonfilter
	);
	char* name;
	char* reason_id;
	ITR = BUF;
	compsky::asciify::asciify(ITR, '{');
	while(compsky::mysql::assign_next_row(RES, &ROW, &name, &reason_id)){
		compsky::asciify::asciify(
			ITR,
			"\"", reason_id, "\":"
				"\"", _f::esc, '"', name, "\""
			","
		);
	}
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, '}', '\0');
	DST = BUF;
}

extern "C"
void get_all_tags(const char* const tagfilter){
	// TODO: Reduce code duplication (virtually identical to get_all_reasons)
	compsky::mysql::query(
		sql::obj,
		RES,
		"SELECT t.name, t.id "
		"FROM tag t "
		"WHERE TRUE ",
		tagfilter
	);
	char* name;
	char* id;
	ITR = BUF;
	compsky::asciify::asciify(ITR, '{');
	while(compsky::mysql::assign_next_row(RES, &ROW, &name, &id)){
		compsky::asciify::asciify(
			ITR,
			"\"", id, "\":"
				"\"", _f::esc, '"', name, "\""
			","
		);
	}
	if(get_index(ITR, BUF) > 1)
		--ITR;
	compsky::asciify::asciify(ITR, '}', '\0');
	DST = BUF;
}
