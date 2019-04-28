default:
	g++ src/main.cpp -o build/main -O3 -lcurl -lb64 -lboost_regex -lmysqlcppconn

debug:
	g++ src/main.cpp -o build/main -g -lcurl -lb64 -lboost_regex -lmysqlcppconn -DDEBUG

j:
	g++ src/json.cpp -o j -O3

man:
	pandoc -s -t man docs/main.md -o man/main.1
