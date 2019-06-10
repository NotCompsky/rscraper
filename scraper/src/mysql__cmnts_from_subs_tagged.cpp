#include <unistd.h> // for write

#include <compsky/mysql/mysql.hpp>
#include <compsky/mysql/query.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky::asciify {
    char* BUF = (char*)malloc(4096);
    int BUF_SZ = 4096;
        
    void ensure_buf_can_fit(size_t n){
        if (BUF_INDX + n  >  BUF_SZ){
            fwrite(BUF, 1, BUF_INDX, stderr);
            BUF_INDX = 0;
        }
    }
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
                    "WHERE t.name IN ('";

constexpr const char* b = 
                "')) T on T.id = s2t.tag_id "
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
                "WHERE rm.name IN ('";

constexpr const char* b = 
            "')) B on B.id = c.reason_matched "
        ") C on C.submission_id = s.id "
    ") D on D.subreddit_id = r.id";
#endif


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

int main(const int argc,  const char** argv){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    
    constexpr static const compsky::asciify::flag::concat::Start c;
    constexpr static const compsky::asciify::flag::concat::End d;
    
    compsky::mysql::query(&RES,
        a,
            c, "','", 3,
                argv+1, argc-1,
            d,
        b
    );
    
    char* subname;
    constexpr static const compsky::asciify::flag::StrLen f;
    char* body;
    uint64_t post_id;
    uint64_t cmnt_id;
    size_t body_sz;
    while (compsky::mysql::assign_next_result(RES, &ROW, &subname, &post_id, &cmnt_id, f, &body_sz, &body)){
        char post_id_str[10];
        char cmnt_id_str[10];
        id2str(post_id, post_id_str);
        id2str(cmnt_id, cmnt_id_str);
        
        compsky::asciify::asciify("https://www.reddit.com/r/",  subname,  "/comments/",  post_id_str,  "/_/",  cmnt_id_str,  '\n');
        
        if (compsky::asciify::BUF_INDX + body_sz > compsky::asciify::BUF_SZ){
            write(1, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
            compsky::asciify::BUF_INDX = 0;
            if (body_sz + 3 + 256 > compsky::asciify::BUF_SZ){
                write(1, body, body_sz);
                continue;
            }
        }
        compsky::asciify::asciify(body, "\n\n\n");
    }
    write(1, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
    compsky::mysql::exit();
}
