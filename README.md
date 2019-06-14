# Usage

See `docs` directory for documentation.

# Building

## Prerequisites

Core Requirements:

    * mysql
    * libmysqlclient
    * [libcompsky](https://github.com/compsky/libcompsky)
    * libcurl
    * libb64
    * rapidjson

Packages required for GUI:

    * libqt5

### Installing

#### Unix

##### Prerequisites

On Ubuntu, they can be installed with:

    `sudo apt install libb64-0d libcurl4 default-libmysqlclient`

Installing MySQL takes some configuration.

    `sudo apt install mysql-client mysql-server`

By default, it seems that `root` can log in to the MySQL server as `root` without a password (through an authentication socket). If that is the case, you can run `sudo rscraper-init` immediately.

#### Windows

##### Prerequisites

Install [libb64](https://sourceforge.net/projects/libb64/files/latest/download). Download the source code, go to `libb64`'s directory, enter `src`, and copy `cencode.c` to this project's `3rdparty/src` folder. Go to `libb64`'s directory, and copy the `include` folder into this project's `3rdparty` folder.

//in `Command Prompt for VS`, and run `cl /LD /Iinclude src/cencode.c /link`. From the `libb64` folder copy `include` into the `rscraper/3rdparty` folder. From the `libb64` folder move the resulting `dll` file into the `rscraper` folder underneath `3rdparty/lib`.

Download the [mysql](https://dev.mysql.com/downloads/connector/c/) binary from the link and follow its instructions, making sure you check the `install C connector` or `install C API bindings` option.

Download `rapidjson`, and copy its `include` directory to `rscraper`'s `3rdparty` directory.

I believe curl has been built into Windows since late 2018.

## Building

Packages required for regex:

    * boost::regex

### Unix

#### Prerequisites

Optional packages required for creating man pages:
    * pandoc

The commands will be similar for other distros, but for Ubuntu specifically:

    `sudo apt install libb64-dev libcurl4-openssl-dev default-libmysqlclient-dev rapidjson-dev`

Then install [libcompsky](https://github.com/compsky/libcompsky) with the linked instructions.

Then navigate to this project's root directory and run:

    `mkdir build`
    `cd build`
    `cmake ..`
    `sudo cmake install`

### Windows

The recommended way of building for Windows is using `MXE` on a Unix system. I have not successfully build it with Visual Studio Code 2015 on my Windows machine.

Actually, I have not compiled the utilities that depend on `libcurl` in MXE either, yet, but it only fails at the last hurdle, linking the binaries.

#### Cross Compiling on Linux with MXE

If you don't already have `libcurl`, navigate to your `MXE` root directory and run `make curl`.

If you don't already have `b64`:
    Download `b64` source code
    Copy the `CMakeLists.txt` file in the `rscraper` repository underneath `3rdparty/cmake` into the root of the `b64`
    mkdir build
    cd build
    `x86_64-w64-mingw32.static-cmake ..`
    make
    sudo make install

Download `rapidjson` and copy its `include` directory into `rscraper/3rdparty`.

Then make and install [libcompsky](https://github.com/compsky/libcompsky) with `x86_64-w64-mingw32.static-cmake`.

Then navigate to `rscraper` root directory and run:

    `x86_64-w64-mingw32.static-cmake ..`
    `make`
    `sudo make install`

But you've probably run into a few million lines of `undefined reference` errors. This is where I decided to try give compiling for Windows a rest. It is definitely possible, and perhaps one brave soul might decide to give it a try some day.

#### VS Code

##### Prerequisites

Install `libcurl`. I recommend getting [the source code](https://curl.haxx.se/dev/source.html) and cmake building (cmake installing) it yourself. Then navigate to `C:/Program Files/CURL/lib`, and copy `libcurl_imp.lib` to `curl.lib`.

Once in the root directory of curl, open up the `Command Prompt for VS` as admin and run:

    `mkdir build`
    `cd build`
    `cmake ..`
    `cmake --build . --config Release --target INSTALL`



##### Building

Right click on the `Command Prompt for VS` and run as admin.

    `mkdir build`
    `cd build`
    `cmake --config Release -G "Visual Studio 15 2017 Win64" ..`
    `cmake --build . --config Release --target INSTALL`

Now, you should get a lot of errors such as `LNK1181: cannot open input file 'compsky_asciify-NOTFOUND.obj' [C:/.../rscrape-cmnts.vcxproj]`. You need to find these `vcxproj` files, and replace all instances of the string `-NOTFOUND` with `.lib`. For instance, the `sed` script `sed -i 's/-NOTFOUND/.lib/g' $(find -name '*.vcxproj')` will do this for you, if you have a bash terminal handy.

###### If things go wrong

The expected output includes lots of warnings - `'fopen' is unsafe`, `conversion from 'size_t' to 'unsigned long'`, `format string requires an argument of 'unsigned long'` etc., which can all be safely ignored.

If `mysql.h` cannot be found, run the `cmake --config Release ...` command again but with `-DWIN_MYSQL_DIR=<value>`, replacing `<value>` with the path to your MySQL server directory, replacing the backslashes with forward slashes - the default is `C:/Program Files/MySQL/MySQL Server 8.0`.

If you get the `library machine type 'x64' conflicts with target machine type 'x86'` warning, you forgot to specify `Win64` in the generator `-G` option before.

The generator must be set to 64 bit otherwise VC will force a x86 build (even if the x64 command prompt is used), and that causes undefined reference (linking) errors.

Note that `--config Release` must be used because `-DCMAKE_BUILD_TYPE` is ignored. Debug builds are not possible in VC builds on Windows because VC believes it can only have the same debug level as libmysqlclient (which is Release).

The directories must be explicitly stated because the find_package command does not find them, even if the FindMySQL.cmake file from the CMake community wiki is copied into the CMake Modules folder. 

#### 

# Advanced Usage

The request delay for Reddit is 1 second. The maximum number of comments we can get with each request is 100. The rate at which new comments are written, we could almost always use a delay of 3 seconds.

To make better use of our request allowance, we can have our programs use a single proxy, which manages the requests from any number of these programs, such that there is at least 1 second between requests sent to reddit.com.

I spent a long time trying to find a program that did this - in the end, it seems that `ncat` (from nmap) is the closest tool, but (as of nmap 7.60) you need to slightly modify the source code for it - in `ncat_proxy.c`, under `ncat_http_server`, replace `fork_handler(i, c);` with `http_server_handler(c); sleep(1);`.
