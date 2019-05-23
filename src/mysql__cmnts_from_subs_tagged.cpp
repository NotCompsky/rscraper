#include <unistd.h> // for write

#include "mymysql.hpp" // for mymysql::*

namespace res1 {
    #include "mymysql_results.hpp" // for ROW, RES, COL, ERR
}


#ifdef SUB2TAG
constexpr const char* a = 
    "SELECT S.name, S.id, c.id, c.content "
    "FROM comment c "
    "JOIN ("
        "SELECT R.name, s.id "
        "FROM submission s "
        "JOIN ( "
            "SELECT r.id, r.name "
            "FROM subreddit r "
            "JOIN ( "
                "SELECT s2t.subreddit_id "
                "FROM subreddit2tag s2t "
                "JOIN ("
                    "SELECT t.id "
                    "FROM tag t "
                    "WHERE t.name IN (";

constexpr const char* b = 
                ")) T on T.id = s2t.tag_id "
            ") S2T on S2T.subreddit_id = r.id "
        ") R on R.id = s.subreddit_id "
    ") S on S.id = c.submission_id";
#else
constexpr const char* a = 
    "SELECT r.name, D.submission_id, D.comment_id, D.content, D.reason "
    "FROM subreddit r "
    "JOIN ( "
        "SELECT C.comment_id, C.content, C.submission_id, s.subreddit_id, C.reason "
        "FROM submission s "
        "JOIN ( "
            "SELECT c.id as 'comment_id', c.content, c.submission_id, B.name as 'reason' "
            "FROM comment c "
            "JOIN ( "
                "SELECT rm.id, rm.name "
                "FROM reason_matched rm "
                "WHERE rm.name IN (";

constexpr const char* b = 
            ")) B on B.id = c.reason_matched "
        ") C on C.submission_id = s.id "
    ") D on D.subreddit_id = r.id";
#endif

constexpr const int BUF_SZ_INIT = 4096;
int BUF_SZ = BUF_SZ_INIT;
char* BUF = (char*)malloc(BUF_SZ_INIT);
int BUF_INDX = 0;


void id2str(unsigned long int id_orig,  char* buf){
    int n_digits = 0;
    unsigned long int id = id_orig;
    while (id != 0){
        ++n_digits;
        id /= 36;
    }
    const int n = n_digits;
    while (id_orig != 0){ // Note that a subreddit id should never be 0
        char digit = id_orig % 36;
        buf[--n_digits] = digit + ((digit<10) ? '0' : 'a' - 10);
        id_orig /= 36;
    }
    buf[n] = 0;
}

int main(const int argc, const char* argv[]){
    mymysql::init(argv[1]);  // Init SQL
    
    StartConcatWithApostrapheAndCommaFlag c;
    EndConcatWithApostrapheAndCommaFlag d;
    
    res1::query(
        a,
            c,
                argv+2, argc-2,
            d,
        b
    );
    
    char* subname;
    SizeOfAssigned body_sz;
    char* body;
    uint64_t post_id;
    uint64_t cmnt_id;
    while (res1::assign_next_result(&subname, &post_id, &cmnt_id, &body_sz, &body)){
        char post_id_str[10];
        char cmnt_id_str[10];
        id2str(post_id, post_id_str);
        id2str(cmnt_id, cmnt_id_str);
        
        asciify("https://www.reddit.com/r/",  subname,  "/comments/",  post_id_str,  "/_/",  cmnt_id_str,  '\n');
        
        if (BUF_INDX + body_sz.size > BUF_SZ){
            write(1, BUF, BUF_INDX);
            BUF_INDX = 0;
            if (body_sz.size + 3 + 256 > BUF_SZ){
                write(1, body, body_sz.size);
                continue;
            }
        }
        asciify(body, "\n\n\n");
    }
    write(1, BUF, BUF_INDX);
    res1::free_result();
    mymysql::exit();
}
