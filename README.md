![Icon](tagger/browser-addon/icons/64.png)

# Description

RScraper is a family of independent tools including a scraper, [browser addon](tagger), and chart generators.

![Taster](https://user-images.githubusercontent.com/30552567/60394819-d453d280-9b21-11e9-8dd9-323ae460b2bf.png)

## Components

* [rtagger addon](tagger) - the browser addon for tagging Reddit users
* [tagger](tagger) - the server for the [browser addon](tagger) addon
* [hub](hub) - a GUI manager for the database and configuring the scraper
* [init](init) - one-off helper tools to initialse the database
* [scraper](scraper) - tool for scraping data from Reddit
* [io](io) - import/export tools (as an alternative to scraping Reddit yourself)
* [man](man) - UNIX man pages
* [utils](utils) - CLI database admin tools

### Tagger

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

Then download the packages you want from the [releases page](releases).

Then see the [configuration guide](INSTALLING_UBUNTU.md#Configuring).

Users of other Debian-based distributions may have to modify the dependency package names before the packages will install (you could use `equivs` if you don't want to modify the `deb` itself). Send a bug report with your platform and a suggestion for the alternative dependency.

If installation still fails for some reason, see [installing on Ubuntu](INSTALLING_UBUNTU.md) (and also make a bug report).

## Windows 10

Not supported yet, but very open to PRs. Some weeks ago it cross-compiled fine, so there shouldn't be many changes to the source code required to build it on or for Windows.

The big hurdle to build for Windows is doing one of the following:

* Modifying CMake to cross-compile on MXE for Windows
* Convert the CMake to `pro` files for `qmake`
* Convert the CMake to work with Visual Studio files

The person who issues a PR to allow building for Windows will get a big recognition at the top of the page here. Create an issue if you want to discuss with me the steps I took in cross-compiling test versions.

# Building

See [BUILDING.md](BUILDING.md)
