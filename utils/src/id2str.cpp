/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "id2str.hpp"

#if (defined DEBUG) || (defined TESTS)
# include <stdio.h> // for printf
# include <stdlib.h> // for exit
#endif


size_t id2str(uint64_t id_orig,  char* buf){
	size_t n_digits = 0;
	uint64_t id = id_orig;
	while (id != 0){
		++n_digits;
		id /= 36;
	}
	const size_t to_return = n_digits;
	while (id_orig != 0){ // Note that a subreddit id should never be 0
		char digit = id_orig % 36;
		buf[--n_digits] = digit + ((digit<10) ? '0' : 'a' - 10);
		id_orig /= 36;
	}
	return to_return;
}

uint64_t str2id(const char* str){
	uint64_t n = 0;
	while (*str != 0){
		n *= (10 + 26);
		if (*str >= '0'  &&  *str <= '9')
			n += *str - '0';
#ifdef DEBUG
		else if (*str < 'a'  ||  *str > 'z'){
			printf("ERROR: Bad alphanumeric: %s\n", str);
			exit(1);
		}
#endif
		else
			n += *str - 'a' + 10;
		++str;
	}
#ifdef TESTS
	if (n < 9186871453714)
		fprintf(stderr, "Warning: %lu is unlikely to be a valid ID\n", n);
#endif
	return n;
}

uint64_t myatoi(const char* str){
	uint64_t n = 0;
	while (*str != 0){
		n *= 10;
		n += *str - '0';
		++str;
	}
	return n;
}

#if defined(ID2STR_EXE) || defined(STR2ID_EXE)
#include <stdio.h>
int main(const int argc,  const char** argv){
	for (auto j = 1;  j < argc;  ++j){
#ifdef ID2STR_EXE
		char str[19 + 1];
		str[id2str(myatoi(argv[j]), str)] = 0;
		printf("%s\n", str);
#else
		uint64_t n = str2id(argv[j]);
		printf("%lu\n", n);
#endif
	}
}
#endif
