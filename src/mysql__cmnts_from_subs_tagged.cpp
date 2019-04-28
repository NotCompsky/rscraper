#include <string.h> // for memcpy, strlen
#include <string> // for std::string
#include <unistd.h> // for write

#include "utils.h" // for count_digits

/* MySQL */
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


sql::Driver* SQL_DRIVER;
sql::Connection* SQL_CON;
sql::Statement* SQL_STMT;
sql::ResultSet* SQL_RES;


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
    SQL_DRIVER = get_driver_instance();
    SQL_CON = SQL_DRIVER->connect(argv[1], argv[2], argv[3]);
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
    
    int i = strlen(STMT);
    for (auto j = 4;  j < argc;  ++j){
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
    
    SQL_RES = SQL_STMT->executeQuery(STMT);
    
    while (SQL_RES->next()){
        /*write(1, URL, strlen(URL));
        write(1, COMMENTS, strlen(COMMENTS));
        write(1, &SLASH, 1);*/
        
        const std::string ssubname = SQL_RES->getString(1);
        const char* subname = ssubname.c_str();
        const unsigned long int post_id = SQL_RES->getUInt64(2);
        const unsigned long int cmnt_id = SQL_RES->getUInt64(3);
        const std::string sbody = SQL_RES->getString(4);
        const char* body = sbody.c_str();
        
        /*
        char post_id_str[9];
        post_id_str[id2str(post_id, post_id_str) - 1] = 0;
        
        char cmnt_id_str[9];
        cmnt_id_str[id2str(cmnt_id, cmnt_id_str) - 1] = 0;
        
        printf("https://www.reddit.com/r/%s/comments/%s//%s\n\n%s\n\n\n", subname, post_id_str, cmnt_id_str, body);
        */
        
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
