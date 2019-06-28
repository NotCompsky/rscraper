# Components

Each subdirectory is its own package (`rscraper-NAME`), and has a README page for overview/details/building/installing.

Note: `rscraper-tagger` is for the server library and executables only. You do not need it if you only wish to use the `rtagger` addon (though you will then need to point it to some server elsewhere).

# Usage

See [hub usage guide](guides/hub.md) for detailed instructions on using `rscraper-hub`.

See [man](man) directory for more generic instructions on using the other programs.

# Installing

To install the `rtagger` browser addon, you do not need to install any of these packages, only the addon (or Javascript script) is necessary. Only the server needs to run the following packages.

## Ubuntu and other Debian-based systems

First install libcompsky:

    regexp="https://github\.com/NotCompsky/libcompsky/releases/download/[0-9]\.[0-9]\.[0-9]/libcompsky-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)\.deb"
    url=$(curl -s https://api.github.com/repos/NotCompsky/libcompsky/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
    wget -O /tmp/libcompsky.deb "$url"
    sudo apt install /tmp/libcompsky.deb

Then set the array of packages you wish to install (`init` is not required but the configuration guide assumes it is installed)

    packages=init man io utils scraper gui tagger

Then run:

    for pkg in $packages; do
        regexp="https://github\.com/NotCompsky/rscraper/releases/download/[0-9]\.[0-9]\.[0-9]/RScraper-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)-$pkg\.deb"
        url=$(curl -s https://api.github.com/repos/NotCompsky/RScraper/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
        wget -O "/tmp/RScraper-$pkg.deb" "$url"
        sudo apt install "/tmp/RScraper-$pkg.deb"
    done

Users of other Debian-based distributions may have to modify the dependency package names before the packages will install (you could use `equivs` if you don't want to modify the `deb` itself). Send a bug report with your platform and a suggestion for the alternative dependency.

If installation still fails for some reason, see [installing on Ubuntu](INSTALLING_UBUNTU.md) (and also make a bug report).

## Windows 10

See [installing on Windows](INSTALLING_WINDOWS.md).
