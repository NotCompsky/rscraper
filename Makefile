default: build/rscrape++

all: build/rscrape++ build/srch-by-reason build/srch-tagged-subs build/rtagged.so man/main.1

build/rscrape++:
	g++ src/rscrape.cpp -o build/rscrape++ -O3 -lcurl -lb64 -lboost_regex -lmysqlcppconn

build/srch-by-reason:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-by-reason -lmysqlcppconn -O3

build/srch-tagged-subs:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-tagged-subs -lmysqlcppconn -O3 -DSUB2TAG

build/rtagged.so:
	g++ src/mysql__rtagged.cpp -o build/rtagged.so -lmysqlcppconn -fPIC -shared -O3

man/main.1:
	pandoc -s -t man docs/main.md -o man/main.1


debug:
	g++ src/rscrape.cpp -o build/rscrape++.d -g -lcurl -lb64 -lboost_regex -lmysqlcppconn -DDEBUG
