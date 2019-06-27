##### MySQL

If you do not wish to host the MySQL server yourself, skip this section.

[Download and install a MySQL community server](https://dev.mysql.com/downloads/mysql/).

During configuration, ensure that `TCP/IP` is enabled. Make a note of the port number and root password.

Then run `sudo rscraper-init`. The default answers are, in order:

    C:\\Users\\YOU\\rscraper_mysql.cfg (no need to escape \\s)
    localhost
    <LEAVE BLANK>
    rscraper
    <YOUR CHOICE>
    rscraper
    3306
    root
    <ROOT PASSWORD>

Then, if you wish to use the scrapers - `rscrape-cmnts` - carry out the prerequisites [here](man/rscraper-init-scraper.1.md) and then run `rscraper-init-scraper`.
