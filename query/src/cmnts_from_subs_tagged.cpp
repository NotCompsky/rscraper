/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include <ctime> // for strftime, localtime, time_t
#include <stdio.h> // for fwrite
#include <stdlib.h> // for malloc

#include <compsky/mysql/mysql.hpp>
#include <compsky/mysql/query.hpp>

#include "id2str.hpp" // for id2str


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky {
    namespace asciify {
        char* BUF = (char*)malloc(80000);
        constexpr static const size_t BUF_SZ = 80000;
            
        void ensure_buf_can_fit(size_t n){
            if (BUF_INDX + n  >  BUF_SZ){
                fwrite(BUF, 1, BUF_INDX, stderr);
                BUF_INDX = 0;
            }
        }
    }
}


#ifdef SUB2TAG
constexpr const char* a = 
    "SELECT S.name, S.id, c.id, c.created_at, c.content, u.name, '' as reason " // Dummy column '' to substitute for 'reason' column
    "FROM comment c, user u "
    "JOIN ("
        "SELECT R.name, s.id "
        "FROM submission s "
        "JOIN ( "
            "SELECT r.id, r.name "
            "FROM subreddit r ";
constexpr const char* a2 = 
            "JOIN ( "
                "SELECT s2t.subreddit_id "
                "FROM subreddit2tag s2t "
                "JOIN ("
                    "SELECT t.id "
                    "FROM tag t "
                    "WHERE t.name IN ('";

constexpr const char* b2 = 
                "')) T on T.id = s2t.tag_id "
            ") S2T on S2T.subreddit_id = r.id ";
constexpr const char* b = 
        ") R on R.id = s.subreddit_id "
    ") S on S.id = c.submission_id "
    "WHERE u.id=c.author_id";
#else
constexpr const char* a = 
    "SELECT r.name, s.id, c.id, c.created_at, c.content, u.name, m.name "
    "FROM subreddit r, submission s, comment c, user u, reason_matched m "
    "WHERE ";
constexpr const char* a2 = 
    "m.name IN ('";

constexpr const char* b2 = 
                "') AND ";
constexpr const char* b = 
    "c.reason_matched=m.id "
    "AND s.id=c.submission_id "
    "AND r.id=s.subreddit_id "
    "AND u.id=c.author_id";
#endif


int main(const int argc,  const char** argv){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    
    constexpr static const compsky::asciify::flag::concat::Start c;
    constexpr static const compsky::asciify::flag::concat::End d;
    
    if (argc == 1)
        compsky::mysql::query(&RES, a, b);
    else
        compsky::mysql::query(&RES,
            a, a2,
                c, "','", 3,
                    argv+1, argc-1,
                d,
            b2, b
        );
    
    char* subname;
    constexpr static const compsky::asciify::flag::StrLen f;
    char* body;
    uint64_t post_id;
    uint64_t cmnt_id;
    size_t body_sz;
    uint64_t t;
    char* username;
    char* reason;
    char dt_buf[200];
    struct tm* dt;
    while (compsky::mysql::assign_next_row(RES, &ROW, &subname, &post_id, &cmnt_id, &t, f, &body_sz, &body, &username, &reason)){
        char post_id_str[10];
        char cmnt_id_str[10];
        post_id_str[id2str(post_id, post_id_str)] = 0;
        cmnt_id_str[id2str(cmnt_id, cmnt_id_str)] = 0;
        
        const time_t tt = t;
        dt = localtime(&tt);
        strftime(dt_buf, sizeof(dt_buf), "%Y %a %b %d %H:%M:%S", dt);
        
        compsky::asciify::asciify("https://www.reddit.com/r/",  subname,  "/comments/",  post_id_str,  "/_/",  cmnt_id_str,  '\n',  dt_buf,  '\t',  reason,  "\tby /u/", username,  '\n');
        
        if (compsky::asciify::BUF_INDX + body_sz > compsky::asciify::BUF_SZ){
            fwrite(compsky::asciify::BUF, 1, compsky::asciify::BUF_INDX, stdout);
            compsky::asciify::BUF_INDX = 0;
            if (body_sz + 3 + 256 > compsky::asciify::BUF_SZ){
                fwrite(body, 1, body_sz, stdout);
                continue;
            }
        }
        compsky::asciify::asciify(body, "\n\n\n");
    }
    fwrite(compsky::asciify::BUF, 1, compsky::asciify::BUF_INDX, stdout);
    compsky::mysql::exit_mysql();
}
