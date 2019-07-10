/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "rscraper/tagger.hpp"

#include <compsky/mysql/query.hpp>

#include <cstring> // for memcpy, strlen
#include <stdlib.h> // for abort

#ifndef DEBUG
# define printf(...)
#else
# include <stdio.h> // for printf // TMP
#endif


MYSQL_RES* RES;
MYSQL_ROW ROW;

extern "C" char* DST = NULL; // alias for BUF

namespace compsky {
    namespace asciify {
        char* BUF;
        constexpr const size_t BUF_SZ = 4096 * 1024;
    }
}


constexpr static const char* id_t2_ = "id-t2_";


constexpr size_t strlen_constexpr(const char* s){
    // GCC strlen is constexpr; this is apparently a bug
    return (*s)  ?  1 + strlen_constexpr(s + 1)  :  0;
}





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

constexpr uint64_t str2id(const char* start,  const char* end){
    // End points to the comma after the id
    uint64_t n = 0;
    for (const char* str = start;  str != end;  ++str){
        n *= (10 + 26);
        if (*str >= '0'  &&  *str <= '9')
            n += *str - '0';
        else
            n += *str - 'a' + 10;
    }
    return n;
}

//static_assert(str2id("6l4z3", 0, 5) == 11063919); // /u/AutoModerator
// Causes error in MXE GCC

//static_assert(n_required_bytes("id-t2_foo,id-t2_bar") == strlen_constexpr("{\"foo\":\"#123456\",\"bar\":\"#123456\"}") + 1);

extern "C"
void init(){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));
    compsky::asciify::BUF = (char*)malloc(compsky::asciify::BUF_SZ);
    if (compsky::asciify::BUF == nullptr)
        abort();
}

extern "C"
void exit_mysql(){
    compsky::mysql::exit_mysql();
}

extern "C"
void csv2cls(const char* csv){
    /*
    The input id-t2_IDSTR,id-t2_IDSTR2,id-t2_IDSTR3, ... maps to {"IDSTR":"#0123456", ... }
    
    id_t2_ cancels out 0123456 on all values seperated by commas
    
    So IDSTR,IDSTR2,IDSTR3 ... maps to {"IDSTR":"#","IDSTR2":"#", ... }
    
    So ,, ... maps to {"":"#","":"#", ... }
    
    So strlen(output)  =  2 + n_commas(csv)*6 + strlen(output)
    */
    
    // Convert id-t2_ABCDEF,id-t2_abcdefg,id-t2_12345  to  colour hex codes
    // The former is longer than the latter, so can reuse the same string as the final output
    // SQL statement might still be longer though, so have to create new string for it
    
    for (auto i = 0;  i < 5;  ++i){
        // Safely skip first prefix bar the last character
        if (*(csv++) == 0){
            compsky::asciify::BUF_INDX = 1;
            goto goto_results;
        }
    }
    
    compsky::asciify::BUF_INDX = 0;
    
    constexpr static const char* stmt_pre = 
        "SELECT A.user_id, SUM(A.c), SUM(A.r), SUM(A.g), SUM(A.b), SUM(A.a), GROUP_CONCAT(A.string) "
        "FROM tag2category t2c "
        "JOIN ( "
            "SELECT S2T.user_id, S2T.tag_id, S2T.c, S2T.c*t.r as r, S2T.c*t.g as g, S2T.c*t.b as b, S2T.c*t.a as a, string "
            "FROM tag t "
            "RIGHT JOIN ( "
                "SELECT U2SCC.user_id, s2t.tag_id, SUM(U2SCC.count) as c, GROUP_CONCAT(U2SCC.name, \" \", U2SCC.count) as string "
                "FROM subreddit2tag s2t "
                "JOIN ("
                    "SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count, s.name "
                    "FROM user2subreddit_cmnt_count u2scc, subreddit s "
                    "WHERE s.id=u2scc.subreddit_id AND u2scc.user_id IN (";
    
    memcpy(compsky::asciify::BUF + compsky::asciify::BUF_INDX,  stmt_pre,  strlen_constexpr(stmt_pre));
    compsky::asciify::BUF_INDX += strlen_constexpr(stmt_pre);
    
    {
    bool current_id_valid = (*csv == '_');
    printf("%c\n", *csv);
    ++csv; // Skip last character of first prefix (no need to check for 0 - done in switch)
    const char* current_id_start = csv;
    while (true){
        switch(*csv){
            case 0:
            case ',':
                if (current_id_valid){
                    const uint64_t id = str2id(current_id_start, csv);
                    
                    compsky::asciify::asciify(id);
                    compsky::asciify::asciify(',');
                }
                for (auto i = 0;  i < 6;  ++i)
                    // Safely skip the prefix ("id-t2_")
                    if (*(++csv) == 0)
                        // Good input would end only on k = 0 (corresponding to 'case 0')
                        // Otherwise, there was not the expected prefix in after the comma
                        goto goto_break;
                current_id_valid = (*csv == '_');
                printf("%c\n", *csv);
                current_id_start = csv + 1; // Start at character after comma
                break;
        }
        ++csv;
    }
    }
    
    goto_break:
    
    if (compsky::asciify::BUF_INDX == strlen_constexpr(stmt_pre)){
        // No valid IDs were found
        DST = "{}";
        return;
    }
    
    --compsky::asciify::BUF_INDX; // Remove trailing comma
    
    {
    constexpr static const char* stmt_post =
                ")) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id "
                "GROUP BY U2SCC.user_id, s2t.tag_id "
            ") S2T ON S2T.tag_id = t.id "
        ") A ON t2c.tag_id = A.tag_id "
        "GROUP BY A.user_id, t2c.category_id";
    memcpy(compsky::asciify::BUF + compsky::asciify::BUF_INDX,  stmt_post,  strlen_constexpr(stmt_post));
    compsky::asciify::BUF_INDX += strlen_constexpr(stmt_post);
    }
    
    printf("QRY: %.*s\n",  compsky::asciify::BUF_INDX,  compsky::asciify::BUF); // TMP
    
    compsky::mysql::query_buffer(&RES, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
    
    
    compsky::asciify::BUF_INDX = 1;
    
    //compsky::asciify::BUF[compsky::asciify::BUF_INDX++] = '{'; We obtain an (erroneous) prefix of "]," in the following loop
    // To avoid adding another branch, we simply skip the first character, and overwrite the second with "{" later
    {
    uint64_t last_id = 0;
    uint64_t id;
    uint64_t n_cmnts;
    double r, g, b, a;
    char* s;
    char id_str[20];
    size_t id_str_len;
    size_t s_len;
    constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
    constexpr static const compsky::asciify::flag::StrLen ff;
    while (compsky::mysql::assign_next_row(RES, &ROW, &id, &n_cmnts, &r, &g, &b, &a, ff, &s_len, &s)){
        const size_t max_new_entry_size = strlen_constexpr("],\"id-t2_abcdefghijklm\":[[\"rgba(255,255,255,1.000)\",\"") + s_len + strlen_constexpr("\"],");
        
        if (compsky::asciify::BUF_INDX + max_new_entry_size + 1 > compsky::asciify::BUF_SZ )
            // +1 is to account for the terminating '}' char.
            break;
        
        if (id != last_id){
            --compsky::asciify::BUF_INDX;  // Overwrite trailing comma left by RGBs
            id_str_len = id2str(id, id_str);
            compsky::asciify::asciify("],\"id-t2_",  ff, id_str, id_str_len,  "\":[");
            last_id = id;
        }
        
        compsky::asciify::asciify(
            "[\"rgba(",
            +(uint8_t)(255.0 * r / (double)n_cmnts),  ',',
            +(uint8_t)(255.0 * g / (double)n_cmnts),  ',',
            +(uint8_t)(255.0 * b / (double)n_cmnts),  ',',
            f, (double)(a / (double)n_cmnts), 3,
            ")\",\"",
            ff, s, s_len,
            "\"],"
        );
    }
    }
    goto_results:
    DST = compsky::asciify::BUF;
    if (compsky::asciify::BUF_INDX != 1){
        compsky::asciify::BUF[--compsky::asciify::BUF_INDX] = ']'; // Overwrite trailing comma
        ++DST; // Skip preceding comma
    } else compsky::asciify::BUF_INDX = 0;
    DST[0] = '{';
    compsky::asciify::BUF[++compsky::asciify::BUF_INDX] = '}';
    compsky::asciify::BUF[++compsky::asciify::BUF_INDX] = 0;
}
