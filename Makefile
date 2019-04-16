default:
	g++ src/main.cpp -o build/main -g -lcurl

man:
	pandoc -s -t man docs/main.md -o man/main.1
