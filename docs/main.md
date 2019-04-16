% RSCRAPER++(1) RSCRAPER++ User Manual
% NotCompsky
% 26 April 2019

# NAME

rscraper++ - Collect posts and comments from Reddit, and write to local SQL database.

# SYNOPSIS
./main *USERNAME* *PASSWORD* *APP_AUTHORISATION_STRING*

# ARGUMENTS
*APP_AUTHORISATION_STRING*
:   {client_id}:{cliend_secret}
    Both of these are assigned to you by Reddit when you register an 'app' with them.

# EXAMPLES
    ./main spez p455w0rd aBCd3F-GH1jKlM:AbcdEfghiJKl-mN0pQr5tUVWxYz
