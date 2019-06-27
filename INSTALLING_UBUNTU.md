Navigate to the [releases](releases) page.

Download the `deb` files for the packages you wish to install. This guide will assume you have installed the `init`, `io`, and `gui` packages.

![Releases page](https://user-images.githubusercontent.com/30552567/60239664-50da7d00-98a6-11e9-80d2-041732229232.png)

Navigate to your Downloads folder, and open in the terminal.

Run `sudo apt install ./RScraper-*.deb`.

![Terminal](https://user-images.githubusercontent.com/30552567/60239671-59cb4e80-98a6-11e9-9102-e156814468d2.png)

Run `sudo rscraper-init` and follow its instructions.

Root privileges are required to use root passwordless authentication for MySQL - which is by default the only way to access new (unconfigured) installs of MySQL community servers.

You can run without root (system) privileges, so long as you specify an account on the SLQ server that has root (SQL) privileges, and include its corresponding password.

No further `rscraper` programs will ask to run with root privileges.

After you have completed the instructions, you must either close and reopen the terminal, or run `export RSCRAPER_MYSQL_CFG=...`, in order to set the environmental variables to continue.

![rscraper-init from rscraper-init package](https://user-images.githubusercontent.com/30552567/60239676-6059c600-98a6-11e9-8074-7cb8da7f31d0.png)

Run `rscraper-hub` to open the GUI.

![rscraper-hub from rscraper-hub package](https://user-images.githubusercontent.com/30552567/60239689-6a7bc480-98a6-11e9-8c1b-74b7cd106a6e.png)

Navigate to the `__IO__` (import export) tab.

![__IO__ tab](https://user-images.githubusercontent.com/30552567/60239720-88e1c000-98a6-11e9-83f2-74064b2f4ffd.png)

Navigate to the link shown, and click on the demo data dump link.

![Data dumps link](https://user-images.githubusercontent.com/30552567/60239723-8ed7a100-98a6-11e9-957d-d5b6746af7c7.png)

Download the folder (as a zip file).

![Dropbox](https://user-images.githubusercontent.com/30552567/60239741-a3b43480-98a6-11e9-897f-b0838c8fc183.png)

Extract the zip.

![Downloads](https://user-images.githubusercontent.com/30552567/60239747-a878e880-98a6-11e9-8a97-690726508ca0.png)

Open the extracted folder in a terminal, and run `rscraper-import`.

![rscraper-import from rscraper-io package](https://user-images.githubusercontent.com/30552567/60239752-b0d12380-98a6-11e9-85b0-595940fe164a.png)

