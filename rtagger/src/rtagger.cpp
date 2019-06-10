#include "rtagger.hpp"

#include <string.h> // for memcpy, strlen
#include <stdio.h> // for printf // TMP

#include <compsky/mysql/query.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;

extern "C" char* DST = NULL; // alias for BUF

namespace compsky::asciify {
    char* BUF = (char*)malloc(4096 * 1024);
    size_t BUF_SZ = 4096 * 1024;
}


constexpr static const char* id_t2_ = "id-t2_";


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

constexpr uint64_t str2id(const char* str,  const size_t start_index,  const size_t end_index_plus_one){
    uint64_t n = 0;
    for (auto i = start_index;  i < end_index_plus_one;  ++i){
        n *= (10 + 26);
        if (str[i] >= '0'  &&  str[i] <= '9')
            n += str[i] - '0';
        else
            n += str[i] - 'a' + 10;
    }
    return n;
}

static_assert(str2id("6l4z3", 0, 5) == 11063919); // /u/AutoModerator



size_t estimated_n_bytes(const char* csv){
    size_t n = 0;
    size_t i = 0;
    while (true){
        if (csv[i] == ','){
            n += 2 + strlen("[\"SUBREDDITNAME\",\"rgba(255,255,255,1.0)\"],")*2; // 2 for "": minus ,  2 spaces for ["SUBREDDITNAME","rgba(255,255,255,1.0)"],
        } else if (csv[i] == 0)
            return 1 + (n+5+11) + 1 + 1; // { ... }\0
        ++i;
        ++n;
    }
}

//static_assert(n_required_bytes("id-t2_foo,id-t2_bar") == strlen("{\"foo\":\"#123456\",\"bar\":\"#123456\"}") + 1);

extern "C"
void init(){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));
}

void enlarge_dst(size_t extra_sz){
    // -1 for terminating null byte
    compsky::asciify::BUF[compsky::asciify::BUF_INDX + 1] = 0; // Necessary to null-terminate for memcpy?
    compsky::asciify::BUF_SZ *= 2;
    compsky::asciify::BUF_SZ += 1000 + extra_sz;
    char* dst = (char*)realloc(compsky::asciify::BUF, compsky::asciify::BUF_SZ);
    if (dst != NULL){
        compsky::asciify::BUF = dst;
        return;
    }
    abort();
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
    
    if (csv[0] == 0){
        compsky::asciify::BUF_INDX = 1;
        goto goto_results;
    }
    
    {
    size_t estimated_requirement = estimated_n_bytes(csv) + 1000;
    if (compsky::asciify::BUF_SZ < estimated_requirement)
        enlarge_dst(estimated_requirement);
    }
    
    compsky::asciify::BUF_INDX = 0;
    
    compsky::asciify::asciify(
        "SELECT A.user_id, SUM(A.c), SUM(A.r), SUM(A.g), SUM(A.b), SUM(A.a), GROUP_CONCAT(A.string) "
        "FROM tag2category t2c "
        "JOIN ( "
            "SELECT S2T.user_id, S2T.tag_id, S2T.c, S2T.c*t.r as r, S2T.c*t.g as g, S2T.c*t.b as b, S2T.c*t.a as a, string "
            "FROM tag t "
            "RIGHT JOIN ( "
                "SELECT U2SCC.user_id, s2t.tag_id, SUM(U2SCC.count) as c, GROUP_CONCAT(U2SCC.name, \" \", U2SCC.count) as string "
                "FROM subreddit2tag s2t "
                "JOIN ( "
                    "SELECT u2scc.user_id, u2scc.subreddit_id, u2scc.count, S.name "
                    "FROM user2subreddit_cmnt_count u2scc "
                    "JOIN ( "
                        "SELECT id, name "
                        "FROM subreddit "
                    ") S ON S.id = u2scc.subreddit_id "
                    "WHERE u2scc.user_id IN ("
    );
    
    {
    size_t i = 6; // Skip first prefix
    size_t j = 6;
    bool last_id_invalid = (csv[i] == 'a');
    while (true){
        switch(csv[i]){
            case 0:
            case ',':
                if (!last_id_invalid){
                    const uint64_t id = str2id(csv, j, i);
                    
                    compsky::asciify::asciify(id);
                    compsky::asciify::asciify(',');
                }
                if (csv[i] == 0)
                    goto goto_break;
                i += 6; // Skip "id-t2_"
                if (csv[i+1] == 'a') // 7th character of "may-invalid"
                    last_id_invalid = true;
                j = i + 1; // Start at character after comma
                break;
        }
        ++i;
    }
    }
    goto_break:
    --compsky::asciify::BUF_INDX; // Remove trailing comma
    
    {
    constexpr static const char* stmt_post =
                ")) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id "
                "GROUP BY U2SCC.user_id, s2t.tag_id "
            ") S2T ON S2T.tag_id = t.id "
        ") A ON t2c.tag_id = A.tag_id "
        "GROUP BY A.user_id, t2c.category_id";
    memcpy(compsky::asciify::BUF + compsky::asciify::BUF_INDX,  stmt_post,  strlen(stmt_post));
    compsky::asciify::BUF_INDX += strlen(stmt_post);
    }
    
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
    constexpr static auto f = compsky::asciify::flag::guarantee::between_zero_and_one_inclusive;
    constexpr static auto ff = compsky::asciify::flag::strlen;
    while (compsky::mysql::assign_next_result(RES, &ROW, &id, &n_cmnts, &r, &g, &b, &a, &s)){
        if (id != last_id){
            --compsky::asciify::BUF_INDX;  // Overwrite trailing comma left by RGBs
            id_str_len = id2str(id, id_str);
            compsky::asciify::asciify("],\"id-t2_",  ff, id_str, id_str_len,  "\":[");
            last_id = id;
        }
        
        if (compsky::asciify::BUF_INDX + strlen(s) + 1000  >=  compsky::asciify::BUF_SZ)
            enlarge_dst(strlen(s) + 1000);
        
        compsky::asciify::asciify(
            "[\"rgba(",
            (uint64_t)(255.0 * r / (double)n_cmnts),  ',',
            (uint64_t)(255.0 * g / (double)n_cmnts),  ',',
            (uint64_t)(255.0 * b / (double)n_cmnts),  ',',
            f, (double)(a / (double)n_cmnts), 3,
            ")\",\"",
            s,
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
    printf("%s\n", DST);
}
