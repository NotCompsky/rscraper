//typedef void StartConcatWithApostrapheAndCommaFlag;
//typedef void EndConcatWithApostrapheAndCommaFlag;

#ifndef __COMPSKY_UTILS__
#define __COMPSKY_UTILS__

#include <sys/types.h> // Linux/GNU
#include <string.h> // for memcpy
#include <inttypes.h> // for u?int[0-9]{1,2}_t

extern char* BUF;
extern int BUF_INDX;








/* Structs */

struct StartPrefixFlag{
    StartPrefixFlag(const char* str,  const int len) : str(str), len(len) {};
    const char* str;
    const int len;
};






/*
Code and description based on work by Zhaomo Yang, of the University of California, who released it into the public domain.


*/
static inline void
memzero_secure(void* data,  size_t len){
  #if defined(_WIN32)
    SecureZeroMemory (data, len);
  #elif defined(__GNUC__) || defined(__clang__)
    memset(data, 0, len);
    __asm__ __volatile__("" : : "r"(data) : "memory");
  #else
    volatile char *p = (volatile char *) data;
    while (len--)
      *p++ = 0;
  #endif
}

/*
typedef void* (*memset_t)(void*, int, size_t);
static volatile memset_t memset_fnct = &memset;
void memzero_secure(void* ptr, size_t len){
    // Same as OPENSSL_cleanse - security described in https://www.usenix.org/system/files/conference/usenixsecurity17/sec17-yang.pdf (page 6)
    // Basically just a trick to confuse the compiler into not optimising the memset call away
    memset_fnct(ptr, 0, len);
}
*/







template<typename T>
int count_digits(T m){
    int i = 0;
    T n = m;
    do {
        n /= 10;
        ++i;
    } while (n != 0);
    // Using do{}while() loop rather than while() loop avoids issue with special case of an input of 0 with the latter
    return i;
};




/* Headers */
template<typename... Args>
void asciify(const char* s,  Args... args);

template<typename... Args>
void asciify(const double dd,  int precision,  Args... args);

template<typename... Args>
void asciify(const char c,  Args... args);

template<typename... Args>
void asciify_concatwithapostrapheandcomma(const char** ptrs,  const int n,  Args... args);

#ifdef _GLIBCXX_VECTOR
template<typename... Args>
void asciify_concatwithapostrapheandcomma(std::vector<const char*> ptrs,  const int n,  Args... args);
#endif

template<typename... Args>
void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  const char* s,  Args... args);




void asciify(const char* s){
    while (*s != 0){
        BUF[BUF_INDX++] = *s;
        ++s;
    }
}

void asciify(const char c){
    BUF[BUF_INDX++] = c;
}

template<typename T>
void asciify_integer(T n){
    auto n_digits = count_digits(n);
    auto i = n_digits;
    BUF_INDX += i;
    do {
        BUF[--BUF_INDX] = '0' + (n % 10);
        n /= 10;
    } while (n != 0);
    BUF_INDX += n_digits;
};
void asciify(uint64_t n){
    asciify_integer(n);
}
void asciify(int64_t n){
    asciify_integer(n);
}
void asciify(uint32_t n){
    asciify_integer(n);
}
void asciify(int32_t n){
    asciify_integer(n);
}
void asciify(uint16_t n){
    asciify_integer(n);
}
void asciify(int16_t n){
    asciify_integer(n);
}
void asciify(uint8_t n){
    asciify_integer(n);
}
void asciify(int8_t n){
    asciify_integer(n);
}





template<typename T,  typename... Args>
void asciify_integer(T n,  Args... args){
    asciify_integer(n);
    asciify(args...);
};

template<typename... Args>
void asciify(uint64_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(int64_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(uint32_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(int32_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(uint16_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(int16_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(uint8_t n,  Args... args){
    asciify_integer(n, args...);
};
template<typename... Args>
void asciify(int8_t n,  Args... args){
    asciify_integer(n, args...);
};



template<typename... Args>
void asciify(const char c,  Args... args){
    BUF[BUF_INDX++] = c;
    asciify(args...);
};

template<typename... Args>
void asciify(const char* s,  Args... args){
    asciify(s);
    asciify(args...);
};




struct FillWithLeadingZerosFlag{};

template<typename T,  typename... Args>
void asciify_fillwithleadingzeros_integer(FillWithLeadingZerosFlag z,  T n,  const int min_digits,  Args... args){
    int n_digits = count_digits(n);
    for (auto i = n_digits;  i < min_digits;  ++i)
        BUF[BUF_INDX++] = '0';
    asciify(n);
    asciify(args...);
};

template<typename... Args>
void asciify(FillWithLeadingZerosFlag z,  uint64_t n,  Args... args){
    asciify_fillwithleadingzeros_integer(z, n, args...);
    asciify(args...);
};
template<typename... Args>
void asciify(FillWithLeadingZerosFlag z,  int32_t n,  Args... args){
    asciify_fillwithleadingzeros_integer(z, n, args...);
    asciify(args...);
};






template<typename T>
void asciify_floaty(T dd){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty(-dd);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    for (auto i = 0;  d > 0;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
};

template<typename T>
void asciify_floaty(T dd,  int precision){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty(-dd, precision);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    for (auto i = 0;  i < precision;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
};

template<typename T>
void asciify_floaty_uptofirstzero(T dd){
    if (dd < 0){
        BUF[BUF_INDX++] = '-';
        return asciify_floaty_uptofirstzero(-dd);
    }
    T d = dd;
    int magnitude = 0;
    uint64_t scale = 1;
    do {
        ++magnitude;
        scale *= 10;
        d /= 10;
    } while (d >= 1);
    
    for (auto i = 0;  i < magnitude;  ++i){
        d *= 10;
        const char m = d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
    
    BUF[BUF_INDX++] = '.';
    
    char m;
    for (auto i = 0;  ;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        if (m == 0)
            break;
        d -= m;
    } while (d > 0);
};

void asciify(){
}

struct Infinity{};
struct UpToFirstZero{};

template<typename... Args>
void asciify(double d,  int precision,  Args... args){
    asciify_floaty(d, precision);
    asciify(args...);
};

template<typename... Args>
void asciify(float f,  int precision,  Args... args){
    asciify_floaty(f, precision);
    asciify(args...);
};

template<typename... Args>
void asciify(double d,  Infinity precision,  Args... args){
    asciify_floaty(d);
    asciify(args...);
};

template<typename... Args>
void asciify(float f,  Infinity precision,  Args... args){
    asciify_floaty(f);
    asciify(args...);
};

template<typename... Args>
void asciify(double d,  UpToFirstZero precision,  Args... args){
    asciify_floaty_uptofirstzero(d);
    asciify(args...);
};
























struct EnsureDoubleBetweenZeroAndOne{
    EnsureDoubleBetweenZeroAndOne(const double d) : value(d) {};
    double value;
};
struct DoubleBetweenZeroAndOne{
    DoubleBetweenZeroAndOne(const double d) : value(d) {};
    double value;
};

void asciify(DoubleBetweenZeroAndOne zno,  int precision){
    double d = zno.value;
    for (auto i = 0;  i < precision;  ++i){
        d *= 10;
        char m = (char)d;
        BUF[BUF_INDX++] = '0' + m;
        d -= m;
    }
};

void asciify(EnsureDoubleBetweenZeroAndOne zno,  int precision){
    double d = zno.value;
    if (d >= 1.0)
        BUF[BUF_INDX++] = '1';
    else if (d <= 0.0)
        BUF[BUF_INDX++] = '0';
    else {
        BUF[BUF_INDX++] = '0';
        BUF[BUF_INDX++] = '.';
        DoubleBetweenZeroAndOne zno_b(d);
        return asciify(zno_b, precision);
    }
};

template<typename... Args>
void asciify(EnsureDoubleBetweenZeroAndOne zno,  int precision,  Args... args){
    asciify(zno, precision);
    asciify(args...);
};

template<typename... Args>
void asciify(DoubleBetweenZeroAndOne zno,  int precision,  Args... args){
    asciify(zno, precision);
    asciify(args...);
};










struct StartConcatWithCommaFlag{};
struct EndConcatWithCommaFlag{};


template<typename... Args>
void asciify(StartConcatWithCommaFlag x,  Args... args){
    asciify_concatwithcomma(args...);
};


void asciify_concatwithcomma(EndConcatWithCommaFlag x){
    BUF[--BUF_INDX] = 0;
}

template<typename... Args>
void asciify_concatwithcomma(const char* s,  Args... args){
    asciify(s);
    BUF[BUF_INDX++] = ',';
    asciify_concatwithcomma(args...);
};

template<typename... Args>
void asciify_concatwithcomma(uint64_t n,  Args... args){
    asciify(n);
    BUF[BUF_INDX++] = ',';
    asciify_concatwithcomma(args...);
};

template<typename... Args>
void asciify_concatwithcomma(const int n,  const char** ptrs,  Args... args){
    for (auto i = 0;  i < n;  ++i){
        asciify(ptrs[i]);
        BUF[BUF_INDX++] = ',';
    }
    asciify_concatwithcomma(args...);
};

template<typename... Args>
void asciify_concatwithcomma(const EnsureDoubleBetweenZeroAndOne zao,  int precision,  Args... args){
    asciify(zao, precision);
    BUF[BUF_INDX++] = ',';
    asciify_concatwithcomma(args...);
};

template<typename... Args>
void asciify_concatwithcomma(EndConcatWithCommaFlag x,  Args... args){
    --BUF_INDX;
    asciify(args...);
};






/*
struct StartEscapeBackslashes{};
struct EndEscapeBackslashes{};

void asciify_startescapebackslashes(const char* s){
    while (*s != 0){
        if (*s == '\\')
            BUF[BUF_INDX++] = '\\';
        BUF[BUF_INDX++] = *s;
        ++s;
    }
};

template<typename... Args>
void asciify_startescapebackslashes(const char* s,  Args... args){
    while (*s != 0){
        BUF[BUF_INDX++] = *s;
        ++s;
    }
    asciify_concatwithapostrapheandcomma(args...);
};
*/






struct EndPrefixFlag{};

void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  const char* s){
    BUF[BUF_INDX++] = '\'';
    
    while(*s != 0){
        if (*s == '\''  ||  *s == '\\')
            BUF[BUF_INDX++] = '\\';
        BUF[BUF_INDX++] = *s;
        ++s;
    }
    
    memcpy(BUF + BUF_INDX,  f.str,  f.len);
    BUF_INDX += f.len;
    
    BUF[BUF_INDX++] = '\'';
    BUF[BUF_INDX++] = ',';
};

template<typename... Args>
void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  const char* s,  Args... args){
    asciify_concatwithapostrapheandcomma(f, s);
    asciify_concatwithapostrapheandcomma(f, args...);
};

template<typename... Args>
void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  const char** ss,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i)
        asciify_concatwithapostrapheandcomma(f, ss[i]);
    
    asciify_concatwithapostrapheandcomma(f, args...);
};

#ifdef _GLIBCXX_VECTOR
template<typename... Args>
void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  const std::vector<const char*> ss,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i)
        asciify_concatwithapostrapheandcomma(f, ss[i]);
    
    asciify_concatwithapostrapheandcomma(f, args...);
};
#endif

template<typename... Args>
void asciify_concatwithapostrapheandcomma(StartPrefixFlag f,  EndPrefixFlag e,  Args... args){
    asciify_concatwithapostrapheandcomma(args...);
};





struct StartConcatWith{
    StartConcatWith(const char* str,  const int len) : str(str), len(len) {};
    const char* str;
    const int len;
};
struct EndConcatWith{};

template<typename... Args>
void asciify(StartConcatWith f,  const char** ptrs,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i){
        asciify(ptrs[i]);
        memcpy(BUF + BUF_INDX,  f.str,  f.len);
        BUF_INDX += f.len;
    }
    
    BUF_INDX -= f.len; // Overwrite trailing concatenation string
    
    asciify(f, args...);
};

#ifdef _GLIBCXX_VECTOR
template<typename... Args>
void asciify(StartConcatWith f,  const std::vector<const char*> ptrs,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i){
        asciify(ptrs[i]);
        memcpy(BUF + BUF_INDX,  f.str,  f.len);
        BUF_INDX += f.len;
    }
    
    BUF_INDX -= f.len; // Overwrite trailing concatenation string
    
    asciify(f, args...);
};
#endif

template<typename... Args>
void asciify(StartConcatWith f,  EndConcatWith e,  Args... args){
    asciify(args...);
};













struct StartConcatWithApostrapheAndCommaFlag{};
struct EndConcatWithApostrapheAndCommaFlag{};


template<typename... Args>
void asciify(StartConcatWithApostrapheAndCommaFlag x,  Args... args){
    asciify_concatwithapostrapheandcomma(args...);
};


void asciify_concatwithapostrapheandcomma(EndConcatWithApostrapheAndCommaFlag x){
    BUF[--BUF_INDX] = 0;
}

void asciify_concatwithapostrapheandcomma(const char* s){
    BUF[BUF_INDX++] = '\'';
    while(*s != 0){
        if (*s == '\''  ||  *s == '\\')
            BUF[BUF_INDX++] = '\\';
        BUF[BUF_INDX++] = *s;
        ++s;
    }
    BUF[BUF_INDX++] = '\'';
    BUF[BUF_INDX++] = ','; // NOTE: This function should never be called as the last asciify function - it should always be succeeded by EndConcatWithApostrapheAndCommaFlag - hence we can have trailing commas
};

template<typename... Args>
void asciify_concatwithapostrapheandcomma(const char* s,  Args... args){
    asciify_concatwithapostrapheandcomma(s);
    asciify_concatwithapostrapheandcomma(args...);
};

template<typename... Args>
void asciify_concatwithapostrapheandcomma(const char** ptrs,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i){
        asciify_concatwithapostrapheandcomma(ptrs[i]);
    }
    asciify_concatwithapostrapheandcomma(args...);
};

#ifdef _GLIBCXX_VECTOR
template<typename... Args>
void asciify_concatwithapostrapheandcomma(const std::vector<const char*> ptrs,  const int n,  Args... args){
    for (auto i = 0;  i < n;  ++i){
        asciify_concatwithapostrapheandcomma(ptrs[i]);
    }
    asciify_concatwithapostrapheandcomma(args...);
};
#endif

template<typename... Args>
void asciify_concatwithapostrapheandcomma(EndConcatWithApostrapheAndCommaFlag x,  Args... args){
    --BUF_INDX;
    asciify(args...);
};

#endif
