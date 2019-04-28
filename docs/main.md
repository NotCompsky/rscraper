% RSCRAPER++(1) RSCRAPER++ User Manual
% NotCompsky
% 26 April 2019

# NAME

rscraper++ - Collect posts and comments from Reddit, and write to local SQL database.

# SYNOPSIS
./main *MYSQL_URL* *MYSQL_USR* *MYSQL_PWD* *REDDIT_USR* *REDDIT_PWD* *APP_AUTHORISATION_STRING*

# ARGUMENTS
*MYSQL_URL*
:   Host of MySQL database
    {protocol}//{host}
    For a locally-hosted database on a Unix systems, it might be **unix:///var/run/mysqld/mysqld.sock**
    For a remote database, it might be **mysql://example.com:33060/mysql**
    This string is passed directly to the MySQL connection object, so please refer to the official MySQL documentation for details.

*MYSQL_USR*
:   Username of MySQL. I recommend creating a seperate user called 'rscraper++' with 'CREATE' and 'SELECT' permissions for the entire 'rscraper' database.

*MYSQL_PWD*
:   Corresponding password of the MySQL user.

*REDDIT_USR*
:   Reddit username

*REDDIT_PWD*
:   Reddit password

*APP_AUTHORISATION_STRING*
:   {client_id}:{cliend_secret}
    Both of these are assigned to you by Reddit when you register an 'app' with them.

# EXAMPLES
    ./main spez p455w0rd aBCd3F-GH1jKlM:AbcdEfghiJKl-mN0pQr5tUVWxYz
