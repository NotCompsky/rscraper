default: src/rscrape.cpp

all: src/rscrape.cpp src/mysql__cmnts_from_subs_tagged.cpp docs/main.md

src/rscrape.cpp:
	g++ src/rscrape.cpp -o build/rscrape++ -O3 -lcurl -lb64 -lboost_regex -lmysqlcppconn

src/mysql__cmnts_from_subs_tagged.cpp:
	g++ src/mysql__cmnts_from_subs_tagged.cpp -o build/srch-tagged-subs -lmysqlcppconn

docs/main.md:
	pandoc -s -t man docs/main.md -o man/main.1


debug:
	g++ src/rscrape.cpp -o build/rscrape++.d -g -lcurl -lb64 -lboost_regex -lmysqlcppconn -DDEBUG
