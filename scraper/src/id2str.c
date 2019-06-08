#include <stdio.h> // for printf

int id2str(unsigned long int id_orig,  char* buf){
    int n_digits = 0;
    unsigned long int id = id_orig;
    while (id != 0){
        ++n_digits;
        id /= 36;
    }
    const int to_return = n_digits;
    while (id_orig != 0){ // Note that a subreddit id should never be 0
        char digit = id_orig % 36;
        buf[--n_digits] = digit + ((digit<10) ? '0' : 'a' - 10);
        id_orig /= 36;
    }
    return to_return;
}

unsigned long int str2id(const char* str){
    unsigned long int n = 0;
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
    //PRINTF("%lu\n", n);
    return n;
}

unsigned long int myatoi(const char* str){
    unsigned long int n = 0;
    while (*str != 0){
        n *= 10;
        n += *str - '0';
        ++str;
    }
    return n;
}

int main(const int argc, const char* argv[]){
    char str[20];
    for (auto j = 1;  j < argc;  ++j){
#ifdef TOSTR
        str[id2str(myatoi(argv[j]), str)] = 0;
        printf("%s\n", str);
#else
        unsigned long int n = str2id(argv[j]);
        printf("%lu\n", n);
#endif
    }
}
