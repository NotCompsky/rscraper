# Building

## Packaging

To build packages for redistribution, append the `-DBUILD_PACKAGES=1` to the cmake commands.

## Dependencies

### Required

In addition to those required for installing.

*   libcurl-dev
*   libmysqlclient-dev
*   rapidjson

#### Ubuntu, Raspbian, and other Debian-derived

    sudo apt install libcurl4-openssl-dev default-libmysqlclient-dev rapidjson-dev

#### Windows (Cross Compiling from Linux)

If you don't already have `libcurl`, navigate to your `MXE` root directory and run `make curl`.

Download `rapidjson` and copy its `include` directory into `rscraper/3rdparty`.

Then make and install [libcompsky](https://github.com/compsky/libcompsky) with `x86_64-w64-mingw32.static-cmake`.

#### Windows (native)

Download `base64.h` and `base64.c` as described in the Unix section, and apply the regex substitutions (presumably manually).

Download the [mysql](https://dev.mysql.com/downloads/connector/c/) binary from the link and follow its instructions, making sure you check the `install C connector` or `install C API bindings` option.

Download `rapidjson`, and copy its `include` directory to `rscraper`'s `3rdparty` directory.

### Regex Matching

*   boost::regex

#### Ubuntu, Raspbian, and other Debian-derived

    sudo apt install libboost-regex-dev

### Man Pages

*   pandoc

#### Ubuntu, Raspbian, and other Debian-derived

    sudo apt install pandoc

## Commands

### Linux, Mac, and other Unix-derived

First, build libcompsky (as development of it is closely tied to this project and changes in the bleeding edge will be intertwined) - [optional for GUI]:

    sudo apt install mariadb-client default-libmysqlclient-dev [qt5-default libqt5widgets5]
    git clone https://github.com/NotCompsky/libcompsky
    cd libcompsky
    mkdir build
    cd build
    cmake ..

Then finally:

    git clone https://github.com/NotCompsky/rscraper
    
    cd rscraper
    mkdir -p 3rdparty/include 3rdparty/src 3rdparty/cmake
    wget -O 3rdparty/include/libb64.h https://raw.githubusercontent.com/cburstedde/libsc/b19431d87224c0d9e89e16f0f8dc381a9e11a1ea/libb64/libb64.h
    wget -O 3rdparty/src/base64.c https://raw.githubusercontent.com/cburstedde/libsc/76db2bce7a2f78d789fe3f13234be752b24c5404/libb64/cencode.c
    
    mkdir build
    cd build
    cmake ..
    sudo cmake install

### Windows (Cross Compiling from Linux)

I've successfully compiled the full project. I haven't successfully ran it on Windows.

Navigate to `rscraper` root directory and run:

    git clone https://github.com/NotCompsky/rscraper
    
    cd rscraper
    mkdir -p 3rdparty/include 3rdparty/src 3rdparty/cmake
    wget -O 3rdparty/include/libb64.h https://raw.githubusercontent.com/cburstedde/libsc/master/5282025f88b0d3d30035782fe048239893a8a9bc/libb64.h
    wget -O 3rdparty/src/base64.c https://raw.githubusercontent.com/cburstedde/libsc/master/5282025f88b0d3d30035782fe048239893a8a9bc/cencode.c
    
    mkdir 3rdparty/cmake
    wget -O 3rdparty/cmake/FindMYSQL.cmake https://gist.github.com/RenatoUtsch/1623340/raw/8d2de77f02b88792516b0c6d357b1dee918f6102/FindMYSQL.cmake
    
    mkdir build
    cd build
    x86_64-w64-mingw32.static-cmake ..
    make
    sudo make install

#### rscrape-cmnts

I had a lot of fun trying to get `rscrape-cmnts` to build on MXE. If you get the `undefined reference to nettle_cnd_memcpy` error, run the following commands:

    cd scraper
    x86_64-w64-mingw32.static-g++ -O3 -DNDEBUG   -Wl,--whole-archive CMakeFiles/rscrape-cmnts.dir/objects.a -Wl,--no-whole-archive  -o rscrape-cmnts.exe -Wl,--out-implib,librscrape-cmnts.dll.a -Wl,--major-image-version,0,--minor-image-version,0 @CMakeFiles/rscrape-cmnts.dir/linklibs.rsp -Wl,-Bstatic -lcurl -Wl,-Bdynamic -Wl,-Bsymbolic-functions -Wl,--as-needed -lssh2 -lgnutls -lidn2 -lnettle -lhogweed -lgcrypt -lgpg-error -lunistring -liconv -lssl -lcrypto -lz -lws2_32 -lcrypt32 -lwldap32 -lgmp -lpthread
    
    cd ..
    
    make

## Possible Issues

If you forget to install these mysql packages and run `cmake` first, you will need to run `cmake` again to avoid `cannot find mysql.h` errors. If that doesn't solve the issue, 

### Cannot find libcompsky_*.so (RUNTIME error)

You need to add `/usr/local/lib` (or equivalent, where libcompsky*.so are installed) to your `LD_LIBRARY_PATH`:

    echo "/usr/local/lib"  |  sudo tee /etc/ld.so.conf.d/99local.conf

Then run `sudo ldconfig`.

If the problem persists, check that you are not overriding `LD_LIBRARY_PATH` in `~/.bashrc` or similar.

### mysql.h: No such file or directory

You probably forgot to install the mysql packages (rerun cmake afterwards).

If that doesn't solve it, just edit the `compsky/mysql/mysql.h` file to change the first line to `#define MYSQL_UNDER_DIR`.
