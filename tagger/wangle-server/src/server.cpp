#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <cstring> // for malloc


typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::AlphaNumeric alphanum;
	constexpr static const compsky::asciify::flag::StrLen strlen;
	//constexpr static const compsky::asciify::flag::MaxBufferSize max_sz;
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
	
	constexpr
	std::string_view reasons_json(){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			#include "headers/Cache-Control/1day.c"
			"\n"
			"{}"
		;
	}
	
	constexpr
	std::string_view tags_json(){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			#include "headers/Cache-Control/1day.c"
			"\n"
			"{}"
		;
	}
}

namespace _method {
	enum {
		GET,
		POST,
		UNKNOWN
	};
}

const char* MYSQL_AUTH_FP;

constexpr
uint64_t str2id(const char* str){
	uint64_t n = 0;
	while (*str != 0){
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
	while(*s != ' '  &&  *s != 0){
		n *= 10;
		n += *s - '0';
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
	
	char* reason_filter;
	char* tag_filter;
	
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
	
	template<typename... Args>
	void asciify(Args... args){
		compsky::asciify::asciify(this->itr,  args...);
		
		*this->itr = 0;
	};
	
	template<typename... Args>
	void mysql_query(Args... args){
		compsky::mysql::query(this->buf, &this->res, args...);
	}
	
	void mysql_query_using_buf(){
		compsky::mysql::query_buffer(&this->res, this->buf, this->buf_indx());
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
		
		this->mysql_query(
			&this->res,
			"SELECT r.name, s.id, c.id, c.created_at "
			"FROM comment c, subreddit r, submission s, reason_matched m "
			"WHERE m.id=", reason_id, " "
			  "AND s.id=c.submission_id "
			  "AND r.id=s.subreddit_id "
			  "AND m.id=c.reason_matched ",
			  this->reason_filter,
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
			created_at = "1565684444";
			this->asciify(
				'[',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					created_at, ',',
					'"', _f::alphanum, submission_id, "/_/", _f::alphanum, comment_id, '"',
				']',
				','
			);
		}
		if(this->buf_indx() != 1)
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view subreddits_given_reason(const char* reason_id_str){
		const uint64_t reason_id = a_to_uint64__space_terminated(reason_id_str);
		
		this->mysql_query(
			&this->res,
			"SELECT r.name, COUNT(c.id)/s2cc.count AS count "
			"FROM subreddit r, submission s, comment c, reason_matched m, subreddit2cmnt_count s2cc "
			"WHERE m.id=", reason_id, " "
			  "AND r.id=s.subreddit_id "
			  "AND s.id=c.submission_id "
			  "AND c.reason_matched=m.id "
			  "AND s2cc.id=r.id "
			  "AND s2cc.count>1000 ",
			  this->reason_filter,
			"GROUP BY r.name "
			"HAVING count>10 "
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
			proportion = "123.456";
			this->asciify(
				'[',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					proportion,
				']',
				','
			);
		}
		if(this->buf_indx() != 1)
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		
		return this->get_buf_as_string_view();
	}
	
	std::string_view subreddits_given_userid(const char* id_str){
		const uint64_t id = str2id(id_str);
		
		/*
		if (unlikely(!is_cached(users, n_users, n_users_log2, id))){
			// WARNING: May be annoying if users cache is not updated often.
			DST = http_err::not_in_database;
			return;
		}
		*/
		
		this->mysql_query(
			&this->res,
			"SELECT u2scc.count, r.name, s2t.tag_id "
			"FROM user u, user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit r "
			"WHERE u.id=", id, " "
			  "AND u2scc.user_id=u.id "
			  "AND r.id=u2scc.subreddit_id "
			  "AND s2t.subreddit_id=u2scc.subreddit_id "
			  "AND t.id=s2t.tag_id ", // for tagfilter - hopefully optimised out if it is just a condition on t.id
			  this->tag_filter,
			"LIMIT 1000"
		);
		
		char* count;
		char* subreddit_name;
		char* tag_id;
		this->reset_buf_index();
		this->asciify(
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
		);
		this->asciify('[');
		while(this->mysql_assign_next_row(&count, &subreddit_name, &tag_id)){
			count = "3";
			tag_id = "4";
			this->asciify(
				'[',
					tag_id,
					'"', _f::esc, '"', subreddit_name, '"', ',',
					count,
				']',
				','
			);
		}
		if(this->buf_indx() != 1)
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		
		return this->get_buf_as_string_view();
	}
	
	constexpr
	size_t generate_user_id_list_string(const char* csv){
		char* const buf_init = this->itr;
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
						
						this->asciify(id);
						this->asciify(',');
					}
					for (auto i = 0;  i < 6;  ++i)
						// Safely skip the prefix ("id-t2_")
						if (*(++csv) == 0)
							// Good input would end only on k = 0 (corresponding to 'case 0')
							// Otherwise, there was not the expected prefix in after the comma
							return (uintptr_t)this->itr - (uintptr_t)buf_init;
					current_id_valid = (*csv == '_');
					printf("%c\n", *csv);
					current_id_start = csv + 1; // Start at character after comma
					break;
			}
			++csv;
		}
	}
	
	std::string_view flairs_given_users(const char* csv){
		/*
		Both 'this->tag_filter' and 'this->reason_filter' are arrays of IDs used to filter the objects that populate the users' tag flair.
		E.g. if this->tag_filter==nullptr, all tags are ignored.
		If this->tag_filter=="", all tags are used.
		If this->tag_filter=="AND id=1", only the tag with ID of 1 is used when generating the flair.
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
		
		for (auto i = 0;  i < 5;  ++i)
			// Safely skip first prefix bar the last character
			if (unlikely(*(csv++) == 0))
				return _r::bad_request;

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
		
		if (this->tag_filter != nullptr){
			this->asciify(_f::strlen, stmt_t_1, std::char_traits<char>::length(stmt_t_1));
			char* const start_of_user_IDs = this->itr;
			const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
			if (n_bytes_of_IDs == 0){
				// No valid IDs were found
				return "{}";
			}
			--this->itr; // Remove trailing comma
			this->asciify(')'); // Close 'WHERE id IN (' condition
			this->asciify(this->tag_filter); // Could be empty string, or "AND t.id IN (...)", etc.
			this->asciify(_f::strlen, stmt_t_2, std::char_traits<char>::length(stmt_t_2));
			if (this->reason_filter != nullptr){
				this->asciify(" UNION ALL SELECT 0, 0, 0, 0, 0, 0, 0, 0 UNION ALL ");
				this->asciify(_f::strlen, stmt_m_1, std::char_traits<char>::length(stmt_m_1));
				this->asciify(_f::strlen, start_of_user_IDs, n_bytes_of_IDs);
				// No need to add closing bracket - copied by the above
				this->asciify(this->reason_filter); // Could be empty string, or "AND t.id IN (...)", etc.
				this->asciify(_f::strlen, stmt_m_2, std::char_traits<char>::length(stmt_m_2));
			}
		} else { // Realistically, this should be 'reasons != nullptr' - though no effort is made to check that this holds
			this->asciify(_f::strlen, stmt_m_1, std::char_traits<char>::length(stmt_m_1));
			const size_t n_bytes_of_IDs = generate_user_id_list_string(csv);
			if (n_bytes_of_IDs == 0){
				// No valid IDs were found
				return "{}";
			}
			--this->itr; // Remove trailing comma
			this->asciify(')');
			this->asciify(this->reason_filter); // Could be empty string, or "AND t.id IN (...)", etc.
			this->asciify(_f::strlen, stmt_m_2, std::char_traits<char>::length(stmt_m_2));
		}
		
		this->mysql_query_using_buf();
		
		this->reset_buf_index();
		++this->itr;
		
		//[ We obtain an (erroneous) prefix of "]," in the following loop
		// These two characters are later overwritten with "[{"
		
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
			
			const size_t max_new_entry_size = std::char_traits<char>::length("],\"id-t2_abcdefghijklm\":[[\"rgba(255,255,255,1.000)\",\"01234567890123456789 01234567890123456789\"],");
			
			if (this->buf_indx() + max_new_entry_size + 1  >  this->buf_sz)
				//{ +1 is to account for the terminating '}' char.
				break;
			
			if (id == last_id){
				this->asciify(',');
			} else {
				this->asciify("],\"id-t2_",  _f::alphanum,  id,  "\":[");
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
		goto_results:
		
		// TODO: Account for cases ((this->tag_filter==nullptr), (this->reason_filter==nullptr))
		
		if (this->buf_indx() == 5)
			return "[{},{}]";
		
		char* DST = this->buf + 1;
		
		const bool first_results_nonempty = (DST[1] == ',');   // Begins with ],"id-t2_
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
						return _r::reasons_json();
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
						return _r::tags_json();
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
													"Content-Length: 78\n"
													"\n"
													#include "favicon.txt"
													, strlen(
														#include "headers/return_code/OK.c"
														#include "headers/Content-Type/ico.c"
														#include "headers/Cache-Control/1day.c"
														"Content-Length: 78\n"
														"\n"
													) + 78
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
	: reason_filter("")
	, tag_filter("")
	{
		this->buf = (char*)malloc(this->buf_sz);
		if(unlikely(this->buf == nullptr))
			// TODO: Replace with compsky::asciify::alloc
			exit(4096);
		
		compsky::mysql::init_auth(MYSQL_AUTH_FP);
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

int main(int argc,  char** argv) {
	folly::Init init(&argc, &argv);
	
	MYSQL_AUTH_FP = getenv("RSCRAPER_MYSQL_CFG");

	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(8080);
	server.waitForStop();

	return 0;
}
