#include <string.h> // for memcpy, strlen
#include <string> // for std::string
#include <unistd.h> // for write

#include "utils.h" // for count_digits
#include "sql_utils.hpp" // for mysu::*


#ifdef SUB2TAG
char STMT[2048] = 
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

const char* STMT_POST = 
                ")) T on T.id = s2t.tag_id "
            ") S2T on S2T.subreddit_id = r.id "
        ") R on R.id = s.subreddit_id "
    ") S on S.id = c.submission_id;";
#else
char STMT[2048] = 
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

const char* STMT_POST = 
            ")) B on B.id = c.reason_matched "
        ") C on C.submission_id = s.id "
    ") D on D.subreddit_id = r.id;";
#endif

char URL[1024] = "https://www.reddit.com/r/0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int URL_PREFIX_LEN = strlen("https://www.reddit.com/r/");
//const char* URL = "https://www.reddit.com/r/";

const char* COMMENTS = "/comments/";

const char NEWLINE = '\n';
const char SLASH = '/';


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

int main(const int argc, const char* argv[]){
    mysu::init(argv[1]);  // Init SQL
    
    int i = strlen(STMT);
    for (auto j = 2;  j < argc;  ++j){
        STMT[i++] = '\'';
        memcpy(STMT + i,  argv[j],  strlen(argv[j]));
        i += strlen(argv[j]);
        STMT[i++] = '\'';
        STMT[i++] = ',';
    }
    --i; // Strip last comma
    
    memcpy(STMT + i,  STMT_POST,  strlen(STMT_POST));
    i += strlen(STMT_POST);
    
    STMT[i] = 0;
    
    write(1, STMT, strlen(STMT));
    
    mysu::SQL_RES = mysu::SQL_STMT->executeQuery(STMT);
    
    while (mysu::SQL_RES->next()){
        const std::string ssubname = mysu::SQL_RES->getString(1);
        const char* subname = ssubname.c_str();
        const unsigned long int post_id = mysu::SQL_RES->getUInt64(2);
        const unsigned long int cmnt_id = mysu::SQL_RES->getUInt64(3);
        const std::string sbody = mysu::SQL_RES->getString(4);
        const char* body = sbody.c_str();
        
        i = URL_PREFIX_LEN;
        
        memcpy(URL + i,  subname,  strlen(subname));
        i += strlen(subname);
        
        memcpy(URL + i,  COMMENTS,  strlen(COMMENTS));
        i += strlen(COMMENTS);
        
        i += id2str(post_id,  URL + i);
        
        URL[i++] = '/';
        URL[i++] = '/';
        
        i += id2str(cmnt_id,  URL + i);
        
        write(1, URL, i);
        
        write(1, &NEWLINE, 1);
        write(1, &NEWLINE, 1);
        write(1, body, strlen(body));
        write(1, &NEWLINE, 1);
        write(1, &NEWLINE, 1);
        write(1, &NEWLINE, 1);
    }
}
