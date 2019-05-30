default: all

all: build/rscrape-cmnts build/srch-by-reason build/srch-tagged-subs build/rtagged.so man/rscrape-cmnts.1 man/rscrape-mods.1



# MySQL Utils #

build/srch-by-reason:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-by-reason -lmysqlclient -O3

build/srch-tagged-subs:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-tagged-subs -lmysqlclient -O3 -DSUB2TAG



# Misc #
build/id2str:
	g++ src/id2str.c -o build/id2str -O3
	g++ src/id2str.c -o build/str2id -O3 -DSTR2ID

# Documentation #

man/rscrape-cmnts.1:
	pandoc -s -t man docs/rscrape-cmnts.1.md -o man/rscrape-cmnts.1

man/rscrape-mods.1:
	pandoc -s -t man docs/rscrape-mods.1.md  -o man/rscrape-mods.1
