#include "FrameDecoder.h"
#include "CStringCodec.h"

#include <folly/init/Init.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>

#include <cstring> // for malloc


typedef wangle::Pipeline<folly::IOBufQueue&,  const char*> RTaggerPipeline;

namespace _r {
	constexpr static const std::string_view not_found =
		#include "headers/return_code/NOT_FOUND.c"
		"\n"
		"Not Found"
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
	
	constexpr
	std::string_view comments_given_reason(const char* s){
		return
			#include "headers/return_code/OK.c"
			#include "headers/Content-Type/json.c"
			"\n"
			"{}"
		;
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
		if(this->buf == nullptr)
			// TODO: Replace with compsky::asciify::alloc
			exit(4096);
		this->itr = this->buf;
	}
		void read(Context* ctx,  const char* msg) override {
			size_t remaining_buf_sz = this->buf_sz;
			this->itr = this->buf; // reset_index()
			const std::string_view r = this->determine_response(msg);
			while(*msg != '\n'  &&  *msg != 0  &&  remaining_buf_sz != 0){
				*(this->itr++) = *(msg++);
				--remaining_buf_sz;
			}
			*this->itr = 0;
			std::cout << ctx->getPipeline()->getTransportInfo()->remoteAddr->getHostStr() << '\t' << this->buf << std::endl;
			write(ctx, r);
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
