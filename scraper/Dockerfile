FROM notcompsky/wangle-static-and-ffmpeg:latest AS intermediate__scraper
ENV PATH=$PATH:/usr/local/x86_64-linux-musl/lib
ENV CC=/usr/local/bin/x86_64-linux-musl-gcc
ENV CXX=/usr/local/bin/x86_64-linux-musl-g++
ENV C_INCLUDE_PATH=/usr/local/x86_64-linux-musl/include
ENV LDFLAGS="-Wl,-Bstatic"
ENV CFLAGS="-static"
ENV CXXFLAGS="-static"
WORKDIR /rscraper
RUN git clone --depth 1 https://github.com/NotCompsky/libcompsky \
	&& mkdir libcompsky/build \
	&& cd libcompsky/build \
	&& cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DMYSQL_IS_UNDER_MARIADB_DIR=1 \
		-DMYSQL_UNDER_DIR_OVERRIDE=1 \
		-DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=/usr/local/include \
		.. \
	&& make install \
	\
	&& git clone --depth 1 https://github.com/Tencent/rapidjson \
	&& mv rapidjson/include/rapidjson /usr/local/include \
	\
	&& git clone --depth 1 https://github.com/NotCompsky/rscraper \
	&& mkdir rscraper/build \
	&& cd rscraper \
	&& mkdir -p 3rdparty/include 3rdparty/src 3rdparty/cmake \
	&& curl -s -L https://raw.githubusercontent.com/cburstedde/libsc/b19431d87224c0d9e89e16f0f8dc381a9e11a1ea/libb64/libb64.h > /usr/local/include/libb64.h \
	&& curl -s -L https://raw.githubusercontent.com/cburstedde/libsc/76db2bce7a2f78d789fe3f13234be752b24c5404/libb64/cencode.c > 3rdparty/src/base64.c \
	&& cd build \
	&& LD_LIBRARY_PATH="/usr/local/lib64:$LD_LIBRARY_PATH" cmake \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=ON \
		../scraper \
	&& ( \
		VERBOSE=1 make \
		|| (\
			( \
				printf "#include <cstddef>\n \
	#include <cstdlib>\n \
	\n\
	extern \"C\" {\
	void my_sha512(char* dst,  char* src,  const size_t slen){\
			abort();\
	}\
	}			" > fix-missing-symbol.monkeypatch.cpp \
				&& /usr/local/bin/x86_64-linux-musl-g++ -flto  -static -O3 -DNDEBUG  -Wl,-Bstatic CMakeFiles/rscrape-cmnts.dir/src/scrape.cpp.o CMakeFiles/rscrape-cmnts.dir/src/filter_comment_body_regexp.cpp.o CMakeFiles/rscrape-cmnts.dir/src/filter_comment_body.cpp.o CMakeFiles/rscrape-cmnts.dir/src/filter_subreddit.cpp.o CMakeFiles/rscrape-cmnts.dir/src/filter_user.cpp.o CMakeFiles/rscrape-cmnts.dir/src/filter_init.cpp.o CMakeFiles/rscrape-cmnts.dir/src/curl_utils.cpp.o CMakeFiles/rscrape-cmnts.dir/src/redditcurl_utils.cpp.o CMakeFiles/rscrape-cmnts.dir/rscraper/libcompsky/build/rscraper/utils/src/id2str.cpp.o CMakeFiles/rscrape-cmnts.dir/rscraper/libcompsky/build/rscraper/shared-src/init_regexp_from_file.cpp.o CMakeFiles/rscrape-cmnts.dir/rscraper/libcompsky/build/rscraper/3rdparty/src/base64.c.o  -o rscrape-cmnts  /usr/local/lib64/libcurl.a /usr/local/lib/mariadb/libmariadbclient.a /usr/lib/libboost_regex.a /usr/local/lib64/libssl.a /usr/local/lib64/libcrypto.a fix-missing-symbol.monkeypatch.cpp \
				&& strip --strip-all rscrape-cmnts \
			) \
		) \
	)
# To extract, spin up a container, and run:
# docker cp <CONTAINER_ID>:/rscraper/libcompsky/build/rscraper/build/rscrape-cmnts /tmp/rscrape-cmnts

FROM alpine:latest
COPY --from=intermediate__scraper /rscraper/build/rscrape-cmnts /rscrape-cmnts
ENTRYPOINT ["/rscrape-cmnts"]
