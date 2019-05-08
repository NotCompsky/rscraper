#!/bin/sh

# Example of proxy server specifically designed to limit the union of all proxied requests to at most 1 request per second

~/bin/tools/nmap-7.60/ncat/ncat localhost 8888

# equivalent to

#ncat -l -vvv --proxy-type http localhost 8888
