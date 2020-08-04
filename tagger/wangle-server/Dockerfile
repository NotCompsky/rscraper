# Copied and pasted, with few changes, from https://github.com/NotCompsky/tagem

FROM notcompsky/wangle-static:latest AS intermediate__server
ENV PATH=$PATH:/usr/local/x86_64-linux-musl/lib
ENV CC=/usr/local/bin/x86_64-linux-musl-gcc
ENV CXX=/usr/local/bin/x86_64-linux-musl-g++
ENV C_INCLUDE_PATH=/usr/local/x86_64-linux-musl/include
ENV LDFLAGS="-Wl,-Bstatic"
ENV CFLAGS="-static"
ENV CXXFLAGS="-static"
WORKDIR /rscraper
# WARNING: -fpermissive is used because Facebook's wangle library currently has a line in its logging library that tries to convert void** to void*
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
	&& git clone --depth 1 --recurse-submodules https://github.com/notcompsky/rscraper \
	&& mkdir rscraper/build \
	&& cd rscraper/build \
	&& LD_LIBRARY_PATH="/usr/local/lib64:$LD_LIBRARY_PATH" cmake \
		-DCMAKE_CXX_FLAGS_RELEASE='-fpermissive' \
		\
		-Dn_cached=1000 \
		\
		-DWHICH_MYSQL_CLIENT=mariadbclient \
		-DCMAKE_BUILD_TYPE=Release \
		-DENABLE_STATIC=ON \
		-Dwangle_DIR=/bob-the-builder/wangle/ \
		-Dfolly_DIR=/bob-the-builder/folly/ \
		-Dfizz_DIR=/bob-the-builder/fizz/ \
		rscraper/tagger/wangle-server \
	&& ( \
		make server \
		|| ( \
			/usr/local/bin/x86_64-linux-musl-g++ -flto -static -fpermissive  -Wl,-Bstatic -s CMakeFiles/server.dir/src/server.cpp.o CMakeFiles/server.dir/src/FrameDecoder.cpp.o CMakeFiles/server.dir/rscraper/tagger/src/tagger.cpp.o  -o server  /usr/local/lib/mariadb/libmariadbclient.a /usr/local/lib/libwangle.a /usr/local/lib64/libcurl.a /usr/local/lib/libfizz.a /usr/local/lib/libfolly.a /usr/local/lib64/libfmt.a /usr/local/lib64/liblz4.a /usr/local/lib64/libzstd.a /usr/local/lib64/libsnappy.a /usr/lib/libdwarf.a /usr/local/lib/libunwind.a /usr/local/lib/libsodium.a  /usr/local/lib64/libssl.a /usr/local/lib64/libcrypto.a /usr/local/lib64/libglog.a /usr/local/lib/libgflags.a /usr/local/lib/libevent.a /usr/local/lib64/libdouble-conversion.a -ldl /usr/lib/librt.a            /usr/lib/libboost_context.a rscraper/tagger/wangle-server/docker/fix-missing-symbol.monkeypatch.cpp \
			&& strip --strip-all server \
		) \
	)

FROM alpine:latest
COPY --from=intermediate__server /rscraper/build/server /rscraper-server
EXPOSE 80
ENTRYPOINT ["/rscraper-server", "p", "80"]
