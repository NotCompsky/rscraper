# Components

Each subdirectory is its own package (`rscraper-NAME`), and has a README page for overview/details/building/installing.

Note: `rscraper-tagger` is for the server library and executables only. You do not need it if you only wish to use the `rtagger` addon (though you will then need to point it to some server elsewhere).

# Usage

See [hub usage guide](guides/hub.md) for detailed instructions on using `rscraper-hub`.

See [man](man) directory for more generic instructions on using the other programs.

# Installing

## Ubuntu and other Debian-based systems

    for pkg in man init io utils scraper tagger gui; do
        regexp="https://github\.com/NotCompsky/rscraper/releases/download/[0-9]\.[0-9]\.[0-9]/RScraper-[0-9]+\.[0-9]+\.[0-9]+-$(dpkg --print-architecture)-$pkg\.deb"
        url=$(curl -s https://api.github.com/repos/NotCompsky/RScraper/releases/latest  |  egrep "$regexp" | sed 's%.*"\(https://.*\)"%\1%g')
        wget -O "/tmp/RScraper-$pkg.deb" "$url"
        sudo apt install "/tmp/RScraper-$pkg.deb"
    done

See [installing on Ubuntu](INSTALLING_UBUNTU.md). Other Debian-based distributions may have to modify the package names of dependencies before the packages will install.

## Windows 10

See [installing on Windows](INSTALLING_WINDOWS.md).
