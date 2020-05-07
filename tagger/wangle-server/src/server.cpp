#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <mutex>
#include <cstring> // for malloc


typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::AlphaNumeric alphanum;
	constexpr static const compsky::asciify::flag::StrLen strlen;
	//constexpr static const compsky::asciify::flag::MaxBufferSize max_sz;
}

namespace _filter {
	static char* EMPTY = "";
	static char* REASONS;
	static char* TAGS;
}

namespace _mysql {
	MYSQL* mysql_obj;
	char buf[512];
	char* auth[6];
	constexpr static const size_t buf_sz = 512; // TODO: Alloc file size
}

namespace _r {
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"Not Found"
	;
	
	constexpr static const std::string_view bad_request =
		#include "headers/return_code/BAD_REQUEST.c"
		"\n"
		"Bad Request"
	;
	
	constexpr
	std::string_view return_static(const char* s){
		switch(*(s++)){
			case 'u':
				switch(*(s++)){
					case '.':
						switch(*(s++)){
							case 'j':
								switch(*(s++)){
									case 's':
										switch(*(s++)){
											case ' ':
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/javascript.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "static/utils.js"
												;
											default: return not_found;
										}
									default: return not_found;
								}
							default: return not_found;
						}
					default: return not_found;
				}
			default: return not_found;
		}
	}
	
	static char buf[4096];
	char* itr = nullptr;
	
	static char* reasons_json;
	static char* tags_json;
	
	void init_json(const char* const tbl_name,  const char* const tbl_alias,  char*& dst,  const char* const qry_filter){
		MYSQL_RES* mysql_res;
		MYSQL_ROW mysql_row;
		
		compsky::mysql::query(
			_mysql::mysql_obj,
			mysql_res,
			buf,
			"SELECT ", tbl_alias, ".name, ", tbl_alias, ".id "
			"FROM ", tbl_name, ' ', tbl_alias, ' ',
			"WHERE TRUE ",
			  qry_filter
		);
		
		constexpr static const char* const _headers =
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			#include "headers/Cache-Control/1day.c"
			"\n"
		;
		
		size_t sz = 0;
		
		char* name;
		char* id;
		
		sz += std::char_traits<char>::length(_headers);
		sz += 1;
		while(compsky::mysql::assign_next_row__no_free(mysql_res, &mysql_row, &name, &id)){
			sz +=
				1 + strlen(id) + 1 + 1 +
					1 + 2*strlen(name) + 1 +
				1
			;
		}
		sz += 1;
		sz += 1;
		
		dst = (char*)malloc(sz);
		char* itr = dst;
		if(unlikely(itr == nullptr))
			exit(4096);
		
		compsky::asciify::asciify(itr, _headers);
		compsky::asciify::asciify(itr, '{');
		mysql_data_seek(mysql_res, 0); // Reset to first result
		while(compsky::mysql::assign_next_row(mysql_res, &mysql_row, &name, &id)){
			compsky::asciify::asciify(
				itr,
				'"', id, '"', ':',
					'"', _f::esc, '"', name, '"',
				','
			);
		}
		if (unlikely(*(itr - 1) == ','))
			// If there was at least one iteration of the loop...
			--itr; // ...wherein a trailing comma was left
		*(itr++) = '}';
		*itr = 0;
	}
}

namespace _method {
	enum {
		GET,
		POST,
		UNKNOWN
	};
}

constexpr
uint64_t str2id(const char* str,  const char terminater){
	uint64_t n = 0;
	while (*str != terminater){
		n *= (10 + 26);
		if (*str >= '0'  &&  *str <= '9')
			n += *str - '0';
		else
			n += *str - 'a' + 10;
		++str;
	}
	return n;
}

constexpr
uint64_t str2id(const char* from,  const char* const to){
	// [from, to)
	uint64_t n = 0;
	while (from != to){
		n *= (10 + 26);
		if (*from >= '0'  &&  *from <= '9')
			n += *from - '0';
		else
			n += *from - 'a' + 10;
		++from;
	}
	return n;
}

constexpr
uint64_t a_to_uint64__space_terminated(const char* s){
	uint64_t n = 0;
	while(*s >= '0'  &&  *s <= '9'){
		n *= 10;
		n += *s - '0';
		++s;
	}
	return n;
}

constexpr
unsigned int which_method(const char*& s){
	switch(*(s++)){
		case 'G':
			switch(*(s++)){
				case 'E':
					switch(*(s++)){
						case 'T':
							switch(*(s++)){
								case ' ':
									return _method::GET;
								default: return _method::UNKNOWN;
							}
						default: return _method::UNKNOWN;
					}
				default: return _method::UNKNOWN;
			}
		default: return _method::UNKNOWN;
	}
}

/*
std::string_view if_http_string(const char* s,  const char* const return_string){
	switch(*(s++)){
		case 'H':
			switch(*(s++)){
				case 'T':
					switch(*(s++)){
						case 'T':
							switch(*(s++)){
								case 'P':
									switch(*(s++)){
										case '/':
											switch(*(s++)){
												case '1':
													switch(*(s++)){
														case '.':
															switch(*(s++)){
																case '1':
																	switch(*(s++)){
																		case '\r':
																		case '\n':
																			return return_string;
																		default: return _r::not_found;
																	}
																default: return _r::not_found;
															}
														default: return _r::not_found;
													}
												default: return _r::not_found;
											}
										default: return _r::not_found;
									}
								default: return _r::not_found;
							}
						default: return _r::not_found;
					}
				default: return _r::not_found;
			}
		default: return _r::not_found;
	}
}
*/

class RTaggerHandler : public wangle::HandlerAdapter<const char*,  const std::string_view> {
  private:
	constexpr static const size_t buf_sz = 2 * 1024 * 1024;
	char* buf;
	char* itr;
	size_t remaining_buf_sz;
	
	static std::mutex mysql_mutex;
	MYSQL_RES* res;
	MYSQL_ROW row;
	
	constexpr
	uintptr_t buf_indx(){
		return (uintptr_t)this->itr - (uintptr_t)this->buf;
	}
	
	constexpr
	void reset_buf_index(){
		this->itr = this->buf;
		this->remaining_buf_sz = this->buf_sz;
	}
	
	inline
	char last_char_in_buf(){
		return *(this->itr - 1);
	}
	
	template<typename... Args>
	void asciify(Args... args){
		compsky::asciify::asciify(this->itr,  args...);
	};
	
	void mysql_query_using_buf(){
		this->mysql_mutex.lock();
		compsky::mysql::query_buffer(_mysql::mysql_obj, this->res, this->buf, this->buf_indx());
		this->mysql_mutex.unlock();
	}
	
	template<typename... Args>
	void mysql_query(Args... args){
		this->reset_buf_index();
		this->asciify(args...);
		this->mysql_query_using_buf();
	}
	
	template<typename... Args>
	bool mysql_assign_next_row(Args... args){
		return compsky::mysql::assign_next_row(this->res, &this->row, args...);
	}
	
	std::string_view get_buf_as_string_view(){
		return std::string_view(this->buf, this->buf_indx());
	}
	
	std::string_view comments_given_reason(const char* const reason_id_str){
		const uint64_t reason_id = a_to_uint64__space_terminated(reason_id_str);
		
		if (reason_id == 0)
			return _r::bad_request;
		
		this->mysql_query(
			"SELECT r.name, s.id, c.id, c.created_at "
			"FROM comment c, subreddit r, submission s, reason_matched m "
			"WHERE m.id=", reason_id, " "
			  "AND s.id=c.submission_id "
			  "AND r.id=s.subreddit_id "
			  "AND m.id=c.reason_matched ",
			  _filter::REASONS,
			" ORDER BY c.created_at DESC "
			"LIMIT 100"
		);
		
		char* subreddit_name;
		uint64_t submission_id;
		uint64_t comment_id;
		char* created_at;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&subreddit_name, &submission_id, &comment_id, &created_at)){
			this->asciify(
				'[',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					created_at, ',',
					'"', _f::alphanum, submission_id, "/_/", _f::alphanum, comment_id, '"',
				']',
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view subreddits_given_reason(const char* reason_id_str){
		const uint64_t reason_id = a_to_uint64__space_terminated(reason_id_str);
		
		if (reason_id == 0)
			return _r::bad_request;
		
		this->mysql_query(
			"SELECT r.name, COUNT(c.id)/s2cc.count AS count "
			"FROM subreddit r, submission s, comment c, reason_matched m, subreddit2cmnt_count s2cc "
			"WHERE m.id=", reason_id, " "
			  "AND r.id=s.subreddit_id "
			  "AND s.id=c.submission_id "
			  "AND c.reason_matched=m.id "
			  "AND s2cc.id=r.id "
			  "AND s2cc.count>1000 ",
			  _filter::REASONS,
			"GROUP BY r.name "
			//"HAVING count>10 "
			"ORDER BY count DESC "
			"LIMIT 100"
		);
		
		char* subreddit_name;
		char* proportion;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&subreddit_name, &proportion)){
			this->asciify(
				'[',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					proportion,
				']',
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view subreddits_given_userid(const char* id_str){
		const uint64_t id = str2id(id_str, ' ');
		
		/*
		if (unlikely(!is_cached(users, n_users, n_users_log2, id))){
			// WARNING: May be annoying if users cache is not updated often.
			DST = http_err::not_in_database;
			return;
		}
		*/
		
		this->mysql_query(
			"SELECT u2scc.count, r.name, GROUP_CONCAT(s2t.tag_id) "
			"FROM user2subreddit_cmnt_count u2scc "
			"JOIN subreddit r ON r.id=u2scc.subreddit_id "
			"JOIN subreddit2tag s2t ON s2t.subreddit_id=r.id "
			"WHERE user_id=", id, ' ',
			_filter::TAGS,
			"GROUP BY r.id "
			"LIMIT 1000"
		);
		
		char* count;
		char* subreddit_name;
		char* tag_ids;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&count, &subreddit_name, &tag_ids)){
			this->asciify(
				'[',
					'"', tag_ids, '"', ',',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					count,
				']',
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view reasons_given_userid(const char* id_str){
		const uint64_t id = str2id(id_str, ' ');
		
		/*
		if (unlikely(!is_cached(users, n_users, n_users_log2, id))){
			// WARNING: May be annoying if users cache is not updated often.
			DST = http_err::not_in_database;
			return;
		}
		*/
		
		this->mysql_query(
			"SELECT m.id, r.name, c.created_at, c.submission_id, c.id "
			"FROM reason_matched m "
			"JOIN comment c ON c.reason_matched=m.id "
			"JOIN submission s ON s.id=c.submission_id "
			"JOIN subreddit r ON r.id=s.subreddit_id "
			"WHERE c.author_id=", id, " ",
			  _filter::REASONS,
			"LIMIT 1000"
		);
		
		char* reason_id;
		char* subreddit_name;
		char* created_at;
		uint64_t submission_id;
		uint64_t comment_id;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&reason_id, &subreddit_name, &created_at, &submission_id, &comment_id)){
			this->asciify(
				'[',
					reason_id, ',',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					created_at, ',',
					'"', _f::alphanum, submission_id, "/_/", _f::alphanum, comment_id, '"',
				']',
				','
			);
		}
		if (this->last_char_in_buf() == ',')
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		*this->itr = 0;
		
		return this->get_buf_as_string_view();
	}
	
	constexpr
	size_t generate_user_id_list_string(const char* csv){
		char* const buf_init = this->itr;
		const char* current_id_start = csv;
		while (true){
			switch(*csv){
				case 0: // Don't expect a 0-terminated string
				case ' ': // This is the expected terminator
				case ',':
					const uint64_t id = str2id(current_id_start, csv);
					this->asciify(id);
					this->asciify(',');
					if (*csv == ' '  ||  *csv == 0)
						return (uintptr_t)this->itr - (uintptr_t)buf_init;
					current_id_start = csv + 1; // Start at character after comma
					break;
			}
			++csv;
		}
	}
	
	std::string_view flairs_given_users__empty(){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"[{},{}]"
		;
	}
	
	std::string_view flairs_given_users(const char* csv){
		/*
		Both '_filter::TAGS' and '_filter::REASONS' are arrays of IDs used to filter the objects that populate the users' tag flair.
		E.g. if _filter::TAGS==nullptr, all tags are ignored.
		If _filter::TAGS=="", all tags are used.
		If _filter::TAGS=="AND id=1", only the tag with ID of 1 is used when generating the flair.
		There is no effort to account for the case where both tags and reasons are null. There is no use case for such a configuration.
		*/
		/*
		The input IDSTR,IDSTR2,IDSTR3, ... maps to {"IDSTR":"#0123456", ... }
		
		id_t2_ cancels out 0123456 on all values seperated by commas
		
		So IDSTR,IDSTR2,IDSTR3 ... maps to {"IDSTR":"#","IDSTR2":"#", ... }
		
		So ,, ... maps to {"":"#","":"#", ... }
		
		So strlen(output)  =  2 + n_commas(csv)*6 + strlen(output)
		*/
		
		// Convert ABCDEF,abcdefg,12345  to  colour hex codes
		// The former is longer than the latter, so can reuse the same string as the final output
		// SQL statement might still be longer though, so have to create new string for it
		
		constexpr static const char* const ok_begin =
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		;

		constexpr static const char* const stmt_t_1 = 
			"SELECT A.user_id, SUM(A.c), SUM(A.c) AS distinctname"
				", SUM(A.r*A.c), SUM(A.g*A.c), SUM(A.b*A.c), SUM(A.a*A.c), A.tag_id "
			"FROM tag2category t2c "
			"JOIN ( "
				"SELECT u2scc.user_id, s2t.tag_id, SUM(u2scc.count) AS c, t.r, t.g, t.b, t.a "
				"FROM user2subreddit_cmnt_count u2scc, subreddit s, subreddit2tag s2t, tag t "
				"WHERE s.id=u2scc.subreddit_id AND s.id=s2t.subreddit_id AND t.id=s2t.tag_id AND u2scc.user_id IN (";

		constexpr static const char* const stmt_t_2 =
				// ")" // Closing bracket added seperately 
				" GROUP BY u2scc.user_id, s2t.tag_id, t.r, t.g, t.b, t.a"
			") A ON t2c.tag_id = A.tag_id "
			"GROUP BY A.user_id, t2c.category_id";

		constexpr static const char* const stmt_m_1 =
			"SELECT c.author_id AS user_id, COUNT(m.id), 1, m.r, m.g, m.b, m.a, m.id "
			"FROM comment c, reason_matched m "
			"WHERE c.reason_matched=m.id AND c.author_id IN (";

		constexpr static const char* const stmt_m_2 =
			/* ) */ " GROUP BY c.author_id, m.name, m.r, m.g, m.b, m.a"; // First ')' is not necessary as it is already copied by 'n_bytes_of_IDs' - because the last trailing comma is recorded by n_bytes_of_IDs as the comma is not stripped before then. This is slightly undesirable only for readability, but the alternative is to decrement BUF_INDX within the generate_user_id_list_string function, which would greatly complicate it.

		this->reset_buf_index();
		
		if (_filter::TAGS != nullptr){
			this->asciify(_f::strlen, stmt_t_1, std::char_traits<char>::length(stmt_t_1));
			char* const start_of_user_IDs = this->itr;
			const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
			if (n_bytes_of_IDs == 0){
				// No valid IDs were found
				return this->flairs_given_users__empty();
			}
			--this->itr; // Remove trailing comma
			this->asciify(')'); // Close 'WHERE id IN (' condition
			this->asciify(_filter::TAGS); // Could be empty string, or "AND t.id IN (...)", etc.
			this->asciify(_f::strlen, stmt_t_2, std::char_traits<char>::length(stmt_t_2));
			if (_filter::REASONS != nullptr){
				this->asciify(" UNION ALL SELECT 0, 0, 0, 0, 0, 0, 0, 0 UNION ALL ");
				this->asciify(_f::strlen, stmt_m_1, std::char_traits<char>::length(stmt_m_1));
				this->asciify(_f::strlen, start_of_user_IDs, n_bytes_of_IDs);
				// No need to add closing bracket - copied by the above
				this->asciify(_filter::REASONS); // Could be empty string, or "AND t.id IN (...)", etc.
				this->asciify(_f::strlen, stmt_m_2, std::char_traits<char>::length(stmt_m_2));
			}
		} else { // Realistically, this should be 'reasons != nullptr' - though no effort is made to check that this holds
			this->asciify(_f::strlen, stmt_m_1, std::char_traits<char>::length(stmt_m_1));
			const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
			if (n_bytes_of_IDs == 0){
				// No valid IDs were found
				return this->flairs_given_users__empty();
			}
			--this->itr; // Remove trailing comma
			this->asciify(')');
			this->asciify(_filter::REASONS); // Could be empty string, or "AND t.id IN (...)", etc.
			this->asciify(_f::strlen, stmt_m_2, std::char_traits<char>::length(stmt_m_2));
		}
		
		*this->itr = 0;
		this->mysql_query_using_buf();
		
		this->reset_buf_index();
		++this->itr;
		
		//[ We obtain an (erroneous) prefix of "]," in the following loop
		// These two characters are later overwritten with "[{"
		
		this->asciify(ok_begin);
		{
		uint64_t last_id = 0;
		uint64_t id;
		uint64_t n_cmnts;
		uint64_t div_rgb_by;
		double r, g, b, a;
		char* tag_or_reason_id;
		size_t id_str_len;
		char* position_to_overwrite_with_open_square_brkt = nullptr;
		constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
		while (this->mysql_assign_next_row(&id, &n_cmnts, &div_rgb_by, &r, &g, &b, &a, &tag_or_reason_id)){
			if (id == 0){
				// i.e. we are in between the real selects in the union
				this->asciify("]}"); //[ ",{" is added afterwards, overwriting the "]," that is otherwise placed in this position}
				position_to_overwrite_with_open_square_brkt = this->itr;
				last_id = 0;
				continue;
			}
			
			const size_t max_new_entry_size = std::char_traits<char>::length("],\"abcdefghijklm\":[[\"rgba(255,255,255,1.000)\",\"01234567890123456789 01234567890123456789\"],");
			
			if (this->buf_indx() + max_new_entry_size + 1  >  this->buf_sz)
				//{ +1 is to account for the terminating '}' char.
				break;
			
			if (id == last_id){
				this->asciify(',');
			} else {
				this->asciify("],\"",  _f::alphanum,  id,  "\":[");
				// The previous line leads to the assertion that first_results_nonempty==(compsky::asciify::BUF[1]==',')
				last_id = id;
			}
			
			this->asciify(
				"[\"rgba(",
				+(uint8_t)(255.0 * r / (double)div_rgb_by),  ',',
				+(uint8_t)(255.0 * g / (double)div_rgb_by),  ',',
				+(uint8_t)(255.0 * b / (double)div_rgb_by),  ',',
				f, (double)(a / (double)div_rgb_by), 3,
				")\",",
				tag_or_reason_id,
				',',
				n_cmnts,
				"]" // Leads to the assertion that secnd_results_nonempty==(*(this->itr-1)==']')
			);
		}
		if (position_to_overwrite_with_open_square_brkt != nullptr){
			position_to_overwrite_with_open_square_brkt[0] = ',';
			position_to_overwrite_with_open_square_brkt[1] = '{';
		}
		}
		
		// TODO: Account for cases ((_filter::TAGS==nullptr), (_filter::REASONS==nullptr))
		
		if (this->buf_indx()  ==  5 + std::char_traits<char>::length(ok_begin))
			return this->flairs_given_users__empty();
		
		char* DST = this->buf + std::char_traits<char>::length(ok_begin) + 1;
		
		const bool first_results_nonempty = (DST[1] == ',');   // Begins with ],"
		const bool secnd_results_nonempty = (*(this->itr-1) == ']'); // Ends   with ]
		if (!first_results_nonempty){
			// Only first results set is empty
			--DST;
			DST[0] = '[';
			DST[1] = '{';
			DST[2] = '}';
			DST[3] = ',';
			DST[4] = '{';
			this->asciify(']', '}', ']');
		} else {
			DST[0] = '[';
			DST[1] = '{';
			
			if (secnd_results_nonempty) {
				// Neither is empty
				this->asciify(']', '}', ']');
			} else {
				// Only second is empty
				this->itr += 2; // Account for position_to_overwrite_with_open_square_brkt
				this->asciify('}', ']');
			}
		}
		*this->itr = 0;
		return this->get_buf_as_string_view();
	}
	
	constexpr
	std::string_view return_api(const char* s){
		switch(*(s++)){
			case 'f':
				switch(*(s++)){
					case '/':
						return flairs_given_users(s);
					default: return _r::not_found;
				}
			case 'm':
				switch(*(s++)){
					case '.':
						// m.json
						return _r::reasons_json;
					case '/':
						switch(*(s++)){
							case 'c':
								switch(*(s++)){
									case '/':
										return this->comments_given_reason(s);
									default: return _r::not_found;
								}
							case 'r':
								switch(*(s++)){
									case '/':
										return this->subreddits_given_reason(s);
									default: return _r::not_found;
								}
							default: return _r::not_found;
						}
					default: return _r::not_found;
				}
			case 't':
				switch(*(s++)){
					case '.':
						// m.json
						return _r::tags_json;
					default: return _r::not_found;
				}
			case 'u':
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case 'r':
								switch(*(s++)){
									case '/':
										return this->subreddits_given_userid(s);
									default: return _r::not_found;
								}
							case 'm':
								// /a/u/m/
								switch(*(s++)){
									case '/':
										return this->reasons_given_userid(s);
									default: return _r::not_found;
								}
							default: return _r::not_found;
						}
					default: return _r::not_found;
				}
			default: return _r::not_found;
		}
	}
	
	constexpr
	std::string_view determine_response(const char* s){
		switch(which_method(s)){
			case _method::GET:
				switch(*(s++)){
					case '/':
						switch(*(s++)){
							case ' ':
								return
									#include "headers/return_code/OK.c"
									#include "headers/Content-Type/html.c"
									#include "headers/Cache-Control/1day.c"
									"\n"
									#include "html/root.html"
								;
							case 'a':
								switch(*(s++)){
									case '/':
										return this->return_api(s);
									default: return _r::not_found;
								}
							case 'f':
								switch(*(s++)){
									case 'a':
										switch(*(s++)){
											case 'v':
												// /favicon.ico
												return std::string_view(
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/ico.c"
													#include "headers/Cache-Control/1day.c"
													"Content-Length: 198\n"
													"\n"
													#include "favicon.txt"
													, std::char_traits<char>::length(
														#include "headers/return_code/OK.c"
														#include "headers/Content-Type/ico.c"
														#include "headers/Cache-Control/1day.c"
														"Content-Length: 198\n"
														"\n"
													) + 198
												);
											default: return _r::not_found;
										}
									case '/':
										switch(*(s++)){
											case ' ':
												// /flairs
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/flairs.html"
												;
											case 'r':
												// /regions
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/flairs_regions.html"
												;
											case 's':
												// /slurs
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/flairs_slurs.html"
												;
											default: return _r::not_found;
										}
									default: return _r::not_found;
								}
							case 'm':
								switch(*(s++)){
									case '/':
										switch(*(s++)){
											case ' ':
												// /m/
												return
													#include "headers/return_code/OK.c"
													#include "headers/Content-Type/html.c"
													#include "headers/Cache-Control/1day.c"
													"\n"
													#include "html/reason.html"
												;
											case 'c':
												switch(*(s++)){
													case '/':
														switch(*(s++)){
															case ' ':
																// /m/c/
																return
																	#include "headers/return_code/OK.c"
																	#include "headers/Content-Type/html.c"
																	#include "headers/Cache-Control/1day.c"
																	"\n"
																	#include "html/comments_given_reason.html"
																;
															default: return _r::not_found;
														}
													default: return _r::not_found;
												}
											case 'r':
												switch(*(s++)){
													case '/':
														switch(*(s++)){
															case ' ':
																// /m/r/
																return
																	#include "headers/return_code/OK.c"
																	#include "headers/Content-Type/html.c"
																	#include "headers/Cache-Control/1day.c"
																	"\n"
																	#include "html/subreddits_given_reason.html"
																;
															default: return _r::not_found;
														}
													default: return _r::not_found;
												}
											default: return _r::not_found;
										}
									default: return _r::not_found;
								}
							case 's':
								switch(*(s++)){
									case '/':
										return _r::return_static(s);
									default: return _r::not_found;
								}
							case 'u':
								switch(*(s++)){
									case '/':
										return
											#include "headers/return_code/OK.c"
											#include "headers/Content-Type/html.c"
											#include "headers/Cache-Control/1day.c"
											"\n"
											#include "html/user_summary.html"
										;
									default: return _r::not_found;
								}
							default: return _r::not_found;
						}
					default: return _r::not_found;
				}
			default: return _r::not_found;
		}
	}
  public:
	RTaggerHandler()
	{
		this->buf = (char*)malloc(this->buf_sz);
		if(unlikely(this->buf == nullptr))
			// TODO: Replace with compsky::asciify::alloc
			exit(4096);
	}
	
	~RTaggerHandler(){
	}
	
		void read(Context* ctx,  const char* const msg) override {
			this->reset_buf_index();
			for(const char* msg_itr = msg;  *msg_itr != 0  &&  *msg_itr != '\n';  ++msg_itr){
				this->asciify(*msg_itr);
			}
			*this->itr = 0;
			std::cout << ctx->getPipeline()->getTransportInfo()->remoteAddr->getHostStr() << '\t' << this->buf << std::endl;
			
			const std::string_view v = this->determine_response(msg);
			write(ctx, v);
			close(ctx);
		}
};
std::mutex RTaggerHandler::mysql_mutex;

class RTaggerPipelineFactory : public wangle::PipelineFactory<RTaggerPipeline> {
	public:
		RTaggerPipeline::Ptr newPipeline(std::shared_ptr<folly::AsyncTransportWrapper> sock) override {
			auto pipeline = RTaggerPipeline::create();
			pipeline->addBack(wangle::AsyncSocketHandler(sock));
			pipeline->addBack(wangle::FrameDecoder());
			pipeline->addBack(wangle::CStringCodec());
			pipeline->addBack(RTaggerHandler());
			pipeline->finalize();
			return pipeline;
		}
};

int s2n(const char* s){
	int n = 0;
	while(*s != 0){
		n *= 10;
		n += *s - '0';
		++s;
	}
	return n;
}

int main(int argc,  char** argv) {
	const int port_n = s2n(argv[1]);
	_filter::REASONS = (argc > 2) ? argv[2] : _filter::EMPTY;
	_filter::TAGS    = (argc > 3) ? argv[3] : _filter::EMPTY;
	
	int dummy_argc = 1;
	folly::Init init(&dummy_argc, &argv);
	
	if (mysql_library_init(0, NULL, NULL))
		throw compsky::mysql::except::SQLLibraryInit();
	
	compsky::mysql::init_auth(_mysql::buf, _mysql::buf_sz, _mysql::auth, getenv("RSCRAPER_MYSQL_CFG"));
	compsky::mysql::login_from_auth(_mysql::mysql_obj, _mysql::auth);
	_r::init_json("reason_matched", "m", _r::reasons_json, _filter::REASONS);
	_r::init_json("tag",            "t", _r::tags_json,    _filter::TAGS);

	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(port_n);
	server.waitForStop();
	
	mysql_close(_mysql::mysql_obj);
	mysql_library_end();
	compsky::mysql::wipe_auth(_mysql::buf, _mysql::buf_sz);

	return 0;
}
