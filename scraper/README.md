<p align="center">
	<h3>Docker Images</h3>
	<a href="https://hub.docker.com/repository/docker/notcompsky/rtagger-scraper/tags"><img src="https://img.shields.io/docker/image-size/notcompsky/rtagger-scraper?label=scraper"/></a>
</p>

# Usage

See [the manuals](../man) for usage information on each tool.

In order to use the scraper, you need to have registered an app with reddit.com.

To use regex matching against comment contents, set the `RSCRAPER_REGEX_FILE` environmental variable to the file path of a file containing the regex to use. Be careful not to include trailing newlines that aren't meant to be a part of your regex. The supported format is the `ECMAScript` syntax, as it uses `boost::regex`. However, named groups are supported on top of that. See [libcompsky](https://github.com/NotCompsky/libcompsky/tree/master/regex) for the specific details on these.

# Advanced Usage

The request delay for Reddit is 1 second. The maximum number of comments we can get with each request is 100. The rate at which new comments are written, we could almost always use a delay of 3 seconds.

To make better use of our request allowance, we can have our programs use a single proxy, which manages the requests from any number of these programs, such that there is at least 1 second between requests sent to reddit.com.

I spent a long time trying to find a program that did this - in the end, it seems that `ncat` (from nmap) is the closest tool, but (as of nmap 7.60) you need to slightly modify the source code for it - in `ncat_proxy.c`, under `ncat_http_server`, replace `fork_handler(i, c);` with `http_server_handler(c); sleep(1);`.
