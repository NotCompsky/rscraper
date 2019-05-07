# Advanced Usage

The request delay for Reddit is 1 second. The maximum number of comments we can get with each request is 100. The rate at which new comments are written, we could almost always use a delay of 3 seconds.

To make better use of our request allowance, we can have our programs use a single proxy, which manages the requests from any number of these programs, such that there is at least 1 second between requests sent to reddit.com.

I spent a long time trying to find a program that did this - in the end, it seems that `ncat` (from nmap) is the closest tool, but (as of nmap 7.60) you need to slightly modify the source code for it - in `ncat_proxy.c`, under `ncat_http_server`, replace `fork_handler(i, c);` with `http_server_handler(c); sleep(1);`.
