rscraper -- A family of tools for scraping and using data from the Reddit API
====================================

![Icon](tagger/browser-addon/icons/64.png)

# Description

RScraper contains tools for scraping data, importing and exporting data sets, and using the data for generating charts and for tagging users via a [browser addon](tagger).

Although based around the data from a scraper, the client, scraper, and database utilities are independent of one another.

# Components

* [rtagger addon](tagger) - the browser addon for tagging Reddit users
* [tagger](tagger) - the server for the [browser addon](rtagger) addon
* [hub](hub) - a GUI manager for the database and configuring the scraper
* [init](init) - one-off helper tools to initialse the database
* [scraper](scraper) - tool for scraping data from Reddit
* [io](io) - import/export tools (as an alternative to scraping Reddit yourself)
* [man](man) - UNIX man pages
* [utils](utils) - CLI database admin tools

## Tagger

To install the `rtagger` browser addon, you do not need to install *any* of these packages; only [the addon (or Javascript script)](tagger) is necessary. Only the server needs to install (and run) the `rscraper-tagger` package.

Even the server doesn't need any packages other than that one, though whoever is managing the server will want to install either the `rscraper-io` or `rscraper-scraper` packages to populate the database, and the `rscraper-gui` package for managing the database, and the `rscraper-init` package to initialise the database.

# Usage

See [hub usage guide](guides/hub.md) for detailed instructions on using `rscraper-hub`.

See [man](man) directory for more generic instructions on using the other programs.

# Installing

## Ubuntu, Raspbian, and other Debian-based systems

First install [libcompsky](https://github.com/NotCompsky/libcompsky):

    regexp="https://github\.com/NotCompsky/libcompsky/releases/download/[0-9]\.[0-9]\.[0-9]/libcompsky-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)\.deb"
    url=$(curl -s https://api.github.com/repos/NotCompsky/libcompsky/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
    wget -O /tmp/libcompsky.deb "$url"
    sudo apt install /tmp/libcompsky.deb

Then set the array of packages you wish to install (`init` is not required but the [configuration guide](INSTALLING_UBUNTU.md#Configuring) assumes it is installed)

    packages=init man io utils scraper gui tagger

Then run:

    for pkg in $packages; do
        regexp="https://github\.com/NotCompsky/rscraper/releases/download/[0-9]\.[0-9]\.[0-9]/RScraper-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)-$pkg\.deb"
        url=$(curl -s https://api.github.com/repos/NotCompsky/RScraper/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
        wget -O "/tmp/RScraper-$pkg.deb" "$url"
        sudo apt install "/tmp/RScraper-$pkg.deb"
    done

Then see the [configuration guide](INSTALLING_UBUNTU.md#Configuring).

Users of other Debian-based distributions may have to modify the dependency package names before the packages will install (you could use `equivs` if you don't want to modify the `deb` itself). Send a bug report with your platform and a suggestion for the alternative dependency.

If installation still fails for some reason, see [installing on Ubuntu](INSTALLING_UBUNTU.md) (and also make a bug report).

## Windows 10

See [installing on Windows](INSTALLING_WINDOWS.md).
