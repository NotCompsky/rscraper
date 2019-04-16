default:
	g++ src/main.cpp -o build/main -g -lcurl -lb64 -DDEBUG

man:
	pandoc -s -t man docs/main.md -o man/main.1
