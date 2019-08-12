#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <compsky/asciify/asciify.hpp>

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <cstring> // for malloc


typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
	constexpr static const compsky::asciify::flag::AlphaNumeric alphanum;
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

constexpr
bool is_number_followed_by_space(const char* s){
	if(*s == 0  ||  *s == ' ')
		return false;
	
	while(*s != ' '  &&  *s != 0){
		if(*s < '0'  ||  *s > '9')
			return false;
		++s;
	}
	
	return true;
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
	
	std::string_view get_buf_as_string_view(){
		return std::string_view(this->buf, this->buf_indx());
	}
	
	std::string_view comments_given_reason(const char* const reason_id){
		if (unlikely(!is_number_followed_by_space(reason_id)))
			return _r::bad_request;
		
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
			subreddit_name = "AskReddit";
			submission_id = 0;
			comment_id = 99;
			created_at = "123456";
			this->asciify(
				'[',
					'"', _f::esc, '"', subreddit_name, '"', ',',
					created_at, ',',
					'"', _f::alphanum, submission_id, "/_/", _f::alphanum, comment_id, '"',
				']',
				','
			);
		if(this->buf_indx() != 1)
			// If there was at least one iteration of the loop...
			--this->itr; // ...wherein a trailing comma was left
		this->asciify(']');
		
		return this->get_buf_as_string_view();
	}
	
	constexpr
	std::string_view subreddits_given_reason(const char* s){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"{}"
		;
	}
	
	constexpr
	std::string_view subreddits_given_userid(const char* s){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"{}"
		;
	}
	
	constexpr
	std::string_view flairs_given_users(const char* s){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"{}"
		;
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
	RTaggerHandler(){
		this->buf = (char*)malloc(this->buf_sz);
		if(unlikely(this->buf == nullptr))
			// TODO: Replace with compsky::asciify::alloc
			exit(4096);
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

	wangle::ServerBootstrap<RTaggerPipeline> server;
	server.childPipeline(std::make_shared<RTaggerPipelineFactory>());
	server.bind(8080);
	server.waitForStop();

	return 0;
}
