default: build/server

build/rtagger.so:
	g++ src/rtagger.cpp -o build/rtagger.so -lmysqlcppconn -fPIC -shared -O3 -I../src

build/server:
	go build -o build/server -ldflags="-s -w" src/server.go
