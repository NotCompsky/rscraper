default: all

all: build/rscrape-cmnts build/srch-by-reason build/srch-tagged-subs build/rtagged.so man/main.1


# Scrapers #

build/rscrape-cmnts:
	g++ src/scrape.cpp -o build/rscrape-cmnts -O3 -lcurl -lb64 -lboost_regex -lmysqlcppconn

build/rscrape-mods:
	g++ src/getmods.cpp -o build/rscrape-mods -O3 -lcurl -lb64 -lmysqlcppconn -DSPIDER


# MySQL Utils #

build/srch-by-reason:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-by-reason -lmysqlcppconn -O3

build/srch-tagged-subs:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-tagged-subs -lmysqlcppconn -O3 -DSUB2TAG

man/main.1:
	pandoc -s -t man docs/main.md -o man/main.1
