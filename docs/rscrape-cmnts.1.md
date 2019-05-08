% RSCRAPE-CMNTS(1) RSCRAPE User Manual
% NotCompsky
% 13 April 2019

# NAME

rscrape-cmnts - Collect comments from Reddit, and write to local SQL database.

# SYNOPSIS
rscrape-cmnts *MYSQL_CONFIG_FILE* *REDDIT_CONFIG_FILE*

# ARGUMENTS

## MYSQL CONFIG FILE

The MySQL config file should contain, in this order, separated by a single newline character \\n (notably NOT the \\r\\n that used to be default in Windows Notepad)

*MYSQL_URL*
:   Host of MySQL database
    {protocol}//{host}
    For a locally-hosted database on a Unix systems, it might be **unix:///var/run/mysqld/mysqld.sock**
    For a remote database, it might be **mysql://example.com:33060/mysql**
    This string is passed directly to the MySQL connection object, so please refer to the official MySQL documentation for details.

*MYSQL_USR*
:   Username of MySQL. I recommend creating a separate user called 'rscraper++' with 'CREATE' and 'SELECT' permissions for the entire 'rscraper' database.

*MYSQL_PWD*
:   Corresponding password of the MySQL user.

## REDDIT CONFIG FILE

*REDDIT_USR*
:   Reddit username

*REDDIT_PWD*
:   Reddit password

*APP_AUTHORISATION_STRING*
:   {client_id}:{cliend_secret}
    Both of these are assigned to you by Reddit when you register an 'app' with them.

*USER_AGENT*
:   User agent string, for instance `myprogram 0.0.1-dev1 (by /u/me)`

*PROXY_URL*
:   Url of proxy to use. Leave blank if no proxy.

# EXAMPLES
    rscrape-cmnts ~/.config/rscraper++/mysql.cfg ~/.config/rscraper++/reddit.cfg

# SEE ALSO

*rscrape-mods(1)*
