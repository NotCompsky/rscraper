#ifndef __MYRU__
#define __MYRU__

#include "error_codes.hpp" // for myerr:*


namespace myru {


unsigned long int id2n_lower(const char* str){
    unsigned long int n = 0;
    while (*str != 0){
        n *= (10 + 26);
        if (*str >= '0'  &&  *str <= '9')
            n += *str - '0';
#ifdef DEBUG
        else if (*str < 'a'  ||  *str > 'z'){
            fprintf(stderr, "Bad alphanumeric: %s\n", str);
            handler(myerr::BAD_ARGUMENT);
        }
#endif
        else
            n += *str - 'a' + 10;
        ++str;
    }
    return n;
}

int slashindx(const char* str){
    int i = 0;
    while (str[i] != '/')
        ++i;
    return i;
}


} // END namespace
#endif
