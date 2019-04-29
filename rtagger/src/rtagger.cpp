#include <string.h> // for memcpy, strlen
#include <string> // for std::string
#include <unistd.h> // for write

#include "utils.h" // for count_digits, itoa_nonstandard

#include "rscraper_utils.hpp" // for sql::*


constexpr const char* STMT_PRE = 
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
                "WHERE u2scc.user_id IN (";

constexpr const char* STMT_POST = 
            ")) U2SCC ON U2SCC.subreddit_id = s2t.subreddit_id "
            "GROUP BY U2SCC.user_id, s2t.tag_id "
        ") S2T ON S2T.tag_id = t.id "
    ") A ON t2c.tag_id = A.tag_id "
    "GROUP BY A.user_id, t2c.category_id;";

constexpr const char* id_t2_ = "id-t2_";


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

constexpr uint64_t str2id(const char* str,  const int start_index,  const int end_index_plus_one){
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


unsigned long int SKIPPED_FIRST_CHAR = 0;
char* DST;

extern "C"
void free_dst(){
    free(DST - SKIPPED_FIRST_CHAR); // Since we added 1 to it before
}

int estimated_n_bytes(const char* csv, int& n_commas){
    int n = 0;
    int i = 0;
    while (true){
        if (csv[i] == ','){
            ++n_commas;
            n += 2 + strlen("[\"SUBREDDITNAME\",\"rgba(255,255,255,1.0)\"],")*2; // 2 for "": minus ,  2 spaces for ["SUBREDDITNAME","rgba(255,255,255,1.0)"],
        } else if (csv[i] == 0)
            return 1 + (n+5+11) + 1 + 1; // { ... }\0
        ++i;
        ++n;
    }
}

//static_assert(n_required_bytes("id-t2_foo,id-t2_bar") == strlen("{\"foo\":\"#123456\",\"bar\":\"#123456\"}") + 1);

extern "C"
void init_mysql(const char* mysql_url,  const char* mysql_usr,  const char* mysql_pwd){
    SQL_CON = SQL_DRIVER->connect(mysql_url, mysql_usr, mysql_pwd);
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
}

size_t enlarge_dst(int i,  size_t n_allocated_bytes){
    // -1 for terminating null byte
    DST[i + 1] = 0; // Necessary to null-terminate for memcpy?
    n_allocated_bytes *= 2;
    n_allocated_bytes += 1000;
    printf("Requesting realloc of %luB\n", n_allocated_bytes);
    char* dst = (char*)realloc(DST, n_allocated_bytes);
    if (dst != NULL){
        DST = dst;
        return n_allocated_bytes;
    }
    // TODO: else out of memory
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
    int i = 6; // Skip first prefix
    int j = 6;
    char stmt[strlen(STMT_PRE) + 2*strlen(csv) + strlen(STMT_POST)]; // 2*strlen(csv) because each id string is at least 7 characters long, and largest integer in base 10 would be 13 digits long, and one comma
    int stmt_i = 0;
    
    memcpy(stmt + stmt_i,  STMT_PRE,  strlen(STMT_PRE));
    stmt_i += strlen(STMT_PRE);
    
    int n_users = 1;
    size_t n_allocated_bytes = estimated_n_bytes(csv, n_users) + 1000;
    
    while (true){
        switch(csv[i]){
            case 0:
            case ',':
                const uint64_t id = str2id(csv, j, i);
                
                stmt_i += itoa_nonstandard(id,  stmt + stmt_i);
                if (csv[i] == 0)
                    goto goto_break;
                stmt[stmt_i++] = ',';
                
                i += 6; // Skip "id-t2_"
                j = i + 1; // Start at character after comma
                break;
        }
        ++i;
    }
    goto_break:
    
    memcpy(stmt + stmt_i,  STMT_POST,  strlen(STMT_POST));
    stmt_i += strlen(STMT_POST);
    
    stmt[stmt_i] = 0;
    
    printf("%s\n", stmt);
    
    SQL_RES = SQL_STMT->executeQuery(stmt);
    
    int k = 0;
    
    DST = (char*)malloc(n_allocated_bytes);
    //DST[k++] = '{'; We obtain an (erroneous) prefix of "]," in the following loop
    // To avoid adding another branch, we simply skip the first character, and overwrite the second with "{" later
    uint64_t last_id = 0;
    while (SQL_RES->next()){
        const uint64_t id = SQL_RES->getUInt64(1);
        const int n_cmnts = SQL_RES->getInt(2);
        
        if (id != last_id){
            DST[k] = ']'; // Overwrite trailing comma left by RGBs
            DST[++k] = ',';
            DST[++k] = '"';
            memcpy(DST + k + 1,  id_t2_,  strlen(id_t2_));
            k += strlen(id_t2_);
            k += id2str(id,  DST + k + 1); // Write ID as alphanumeric string
            DST[++k] = '"';
            DST[++k] = ':';
            DST[++k] = '[';
            last_id = id;
        }
        DST[++k] = '[';
        DST[++k] = '"';
        DST[++k] = 'r';
        DST[++k] = 'g';
        DST[++k] = 'b';
        DST[++k] = 'a';
        DST[++k] = '(';
        for (auto c = 3;  c < 6;  ++c){
            k += itoa_nonstandard((uint64_t)(255.0d * SQL_RES->getDouble(c) / n_cmnts),  DST + k + 1);
            DST[++k] = ',';
        }
        const double a = SQL_RES->getDouble(6) / n_cmnts;
        char a1;
        char a2;
        if (a == 1.0d){
            a1 = '1';
            a2 = '0';
        } else {
            a1 = '0';
            a2 = '0' + (unsigned char)(10*a);
        }
        DST[++k] = a1;
        DST[++k] = '.';
        DST[++k] = a2;
        
        DST[++k] = ')';
        DST[++k] = '"';
        DST[++k] = ',';
        
        
        DST[++k] = '"';
        
        const std::string ss = SQL_RES->getString(7);
        const char*        s = ss.c_str();
        
        if (k + strlen(s) + 1000  >=  n_allocated_bytes)
            n_allocated_bytes = enlarge_dst(k,  n_allocated_bytes + strlen(s));
        
        memcpy(DST + k + 1,  s,  strlen(s));
        k += strlen(s);
        
        DST[++k] = '"';
        DST[++k] = ']';
        
        
        DST[++k] = ',';
        
        DST[k+1] = 0;
    }
    SKIPPED_FIRST_CHAR = 0;
    if (k != 0){
        DST[k] = ']';
        --k; // Clear trailing comma
        ++DST; // Skip first character
        SKIPPED_FIRST_CHAR = 1;
    }
    DST[0] = '{';
    DST[++k] = '}';
    DST[++k] = 0;
}
