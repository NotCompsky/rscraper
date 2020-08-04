<p align="center">
	<img src="tagger/browser-addon/icons/64.png"/>
	<h1 align="center">rscraper</h1>
</p>

<p align="center">
	<a href="LICENSE"><img src="https://img.shields.io/github/license/NotCompsky/rscraper"/></a>
	<a href="https://github.com/NotCompsky/rscraper/releases"><img src="https://img.shields.io/github/v/release/NotCompsky/rscraper"/></a>
	<a href="https://circleci.com/gh/NotCompsky/rscraper"><img src="https://circleci.com/gh/NotCompsky/rscraper.svg?style=shield"/></a>
	<a href="https://github.com/NotCompsky/rscraper/graphs/commit-activity"><img src="https://img.shields.io/github/commit-activity/w/NotCompsky/rscraper"/>
	<a href="https://github.com/NotCompsky/rscraper/graphs/contributors"><img src="https://img.shields.io/github/contributors/NotCompsky/rscraper"></a>
	<a href="https://discord.gg/DnD7RJA"><img src="https://img.shields.io/discord/736649679575580814?label=Discord"></a>
	<a href="https://api.codacy.com/project/badge/Grade/9ee8e250c8f842559559e7a509e80971"><img src="https://www.codacy.com/app/NotCompsky/rscraper?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=NotCompsky/rscraper&amp;utm_campaign=Badge_Grade"></a>
</p>

<p align="center">
	<h3>Docker Images</h3>
	<a href="https://hub.docker.com/repository/docker/notcompsky/rscrape-cmnts/tags"><img src="https://img.shields.io/docker/image-size/notcompsky/rscrape-cmnts?label=scraper"/></a>
	<a href="https://hub.docker.com/repository/docker/notcompsky/rtagger-server/tags"><img src="https://img.shields.io/docker/image-size/notcompsky/rtagger-server?label=server"/></a>
</p>

## Description

RScraper is a family of independent tools including a scraper, [browser addon](tagger), and chart generators.

![Taster](https://user-images.githubusercontent.com/30552567/60394819-d453d280-9b21-11e9-8dd9-323ae460b2bf.png)

### Components

*   [rtagger addon](tagger) - the browser addon for tagging Reddit users
*   [tagger](tagger) - the server for the [browser addon](tagger) addon
*   [hub](hub) - a GUI manager for the database and configuring the scraper
*   [init](init) - one-off helper tools to initialse the database
*   [scraper](scraper) - tool for scraping data from Reddit
*   [io](io) - import/export tools (as an alternative to scraping Reddit yourself)
*   [man](man) - UNIX man pages
*   [utils](utils) - CLI database admin tools

#### Tagger

To install the `rtagger` browser addon, you do not need to install *any* of these packages; only [the addon (or Javascript script)](tagger) is necessary. Only the server needs to install (and run) the `rscraper-tagger` package.

Even the server doesn't need any packages other than that one, though whoever is managing the server will want to install either the `rscraper-io` or `rscraper-scraper` packages to populate the database, and the `rscraper-gui` package for managing the database, and the `rscraper-init` package to initialise the database.

## Usage

See [hub usage guide](guides/hub.md) for detailed instructions on using `rscraper-hub`.

See [man](man) directory for more generic instructions on using the other programs.

## Platforms

Debian-based systems can use the `deb` installer packages in the [releases page](https://github.com/NotCompsky/rscraper/releases) - `amd64` for `x86_64` systems (most laptops and desktops), `armhf` for 64bit arm (e.g. Raspberry Pi). I have tested it on `Ubuntu`, `Raspbian`, and `Debian`. Other (up to date) Debian-based distros should also work.

It should work on MacOS and other Linux distros too. I just don't have access to such systems, so currently the only option for these systems is to [build](BUILDING.md) from source.

Windows support is pending someone more knowledgeable about Windows builds helping out.

## Installing

### Ubuntu, Raspbian, and other Debian-based systems

First install [libcompsky](https://github.com/NotCompsky/libcompsky):

    regexp="https://github\.com/NotCompsky/libcompsky/releases/download/[0-9]+\.[0-9]+\.[0-9]+/libcompsky-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)\.deb"
    url=$(curl -s https://api.github.com/repos/NotCompsky/libcompsky/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
    wget -O /tmp/libcompsky.deb "$url"
    sudo apt install /tmp/libcompsky.deb

Then set the array of packages you wish to install (`init` is not required but the [configuration guide](INSTALLING_UBUNTU.md#Configuring) assumes it is installed)

Then download the packages you want from the [releases page](https://github.com/NotCompsky/rscraper/releases).

Then see the [configuration guide](INSTALLING_UBUNTU.md#Configuring).

If installation still fails for some reason, see [installing on Ubuntu](INSTALLING_UBUNTU.md) (and also make a bug report).

### Windows 10

Not supported yet, but very open to PRs. Some weeks ago it cross-compiled fine, so there shouldn't be many changes to the source code required to build it on or for Windows.

The big hurdle to build for Windows is doing one of the following:

* Modifying CMake to cross-compile on MXE for Windows
* Convert the CMake to `pro` files for `qmake`
* Convert the CMake to work with Visual Studio files

The person who issues a PR to allow building for Windows will get a big recognition at the top of the page here. Create an issue if you want to discuss with me the steps I took in cross-compiling test versions.

## Building

See [BUILDING.md](BUILDING.md)

## ROADMAP

This is still in active development, so expect quite a few things to change.

What should stay the same is the database structure. Purely aesthetic changes - such as the names of columns - will not be made.

Backwards-incompatible changes are very unlikely in the database structure (defined in [init.sql](init/src/init.sql)), tagger, init and io, and unlikely in utils.

Features may be added in particular to `rscraper-hub`.
