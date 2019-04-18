default:
	g++ src/main.cpp -o build/main -g -lcurl -lb64 -DDEBUG

j:
	g++ src/json.cpp -o j -O3

man:
	pandoc -s -t man docs/main.md -o man/main.1
