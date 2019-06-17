/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include <stdio.h> // for fwrite
#include <stdlib.h> // for malloc

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/print.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky {
    namespace asciify {
        char* BUF = (char*)malloc(4096 * 16);
        constexpr static const size_t BUF_SZ = 4096 * 16;
            
        void ensure_buf_can_fit(size_t n){
            if (BUF_INDX + n  >  BUF_SZ){
                fwrite(BUF, 1, BUF_INDX, stderr);
                BUF_INDX = 0;
            }
        }
    }
}

namespace _f {
    constexpr static const compsky::asciify::flag::concat::Start start;
    constexpr static const compsky::asciify::flag::concat::End end;
}

bool contains(const char** ls,  const int n,    const char* s){
    const size_t slen = strlen(s);
    for (auto i = 0;  i < n;  ++i)
        if (strncmp(ls[i], s, slen) == 0)
            return true;
    return false;
}

int main(int argc,  const char** argv){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    
    FILE* f;
    uint64_t count, id, subreddit_id, tag_id, user_id, category_id;
    char* name;
    char* name2;
    
    ++argv;
    --argc;
    
    
    // Arg Parser
    std::vector<const char*> categories_wl_; // whitelist
    //std::vector<const char*> categories_bl; // blacklist
    while(argc != 0){
        const char* s = *argv;
        if (s[2] != 0  ||  s[0] != '-')
            break;
        switch(s[1]){
            /*case 'C':
                categories_bl.push_back(s);
                break;*/
            case 'c':
                categories_wl_.push_back(*(++argv));
                --argc;
                break;
            default:
                fprintf(stderr, "Unrecognised option: %s\n", s);
                exit(1);
        }
        ++argv;
        --argc;
    }
    const char** categories_wl = &categories_wl_[0]; // compsky::asciify::asciify throws errors with std::vector when concatenating here, not sure why.
    const auto categories_wl_size = categories_wl_.size();
    
    /* No dependencies on other tables */
    
    if (argc == 0  ||  contains(argv, argc, "user")){
        f = fopen("user.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT id, name FROM user");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT u.id, u.name FROM user u, user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag2category t2c, category c WHERE u.id=u2scc.user_id AND u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t2c.tag_id AND t2c.category_id=c.id AND c.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        while(compsky::mysql::assign_next_row(RES, &ROW, &id, &name))
            compsky::asciify::write(f,  id, '\t', name, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit")){
        f = fopen("subreddit.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT id, name FROM subreddit");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT s.id, s.name FROM subreddit s, subreddit2tag s2t, tag2category t2c, category c WHERE s.id=s2t.subreddit_id AND s2t.tag_id=t2c.tag_id AND t2c.category_id=c.id AND c.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        while(compsky::mysql::assign_next_row(RES, &ROW, &id, &name))
            compsky::asciify::write(f,  id, '\t', name, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag")){
        double r, g, b, a;
        f = fopen("tag.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT name, CONCAT_WS(',', r, g, b, a) FROM tag");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT t.name, CONCAT_WS(',', r, g, b, a) FROM tag t, tag2category t2c, category c WHERE t.id=t2c.tag_id AND t2c.category_id=c.id AND c.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f_inc;
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &name2))
            compsky::asciify::write(f, name, '\t', name2, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "category")){
        f = fopen("category.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT name FROM category");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT name FROM category WHERE name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        while(compsky::mysql::assign_next_row(RES, &ROW, &name))
            compsky::asciify::write(f,  name, '\n');
        fclose(f);
    }
    
    /* No dependencies on previous import data (i.e. just using absolute IDs) */
    
    if (argc == 0  ||  contains(argv, argc, "user2subreddit_cmnt_count")){
        f = fopen("user2subreddit_cmnt_count.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT user_id, subreddit_id, count FROM user2subreddit_cmnt_count");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT user_id, u2scc.subreddit_id, count FROM user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag2category t2c, category c WHERE u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t2c.tag_id AND t2c.category_id=c.id AND c.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        while(compsky::mysql::assign_next_row(RES, &ROW, &user_id, &subreddit_id, &count))
            compsky::asciify::write(f,  user_id, '\t', subreddit_id, '\t', count, '\n');
        fclose(f);
    }
    
    /* Name-to-name tables */
    
    if (argc == 0  ||  contains(argv, argc, "tag2category")){
        f = fopen("tag2category.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT B.name, C.name FROM tag2category A, tag B, category C WHERE B.id=A.tag_id AND C.id=A.category_id");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT B.name, C.name FROM tag2category A, tag B, category C WHERE B.id=A.tag_id AND C.id=A.category_id AND C.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &name2))
            compsky::asciify::write(f,  name, '\t', name2, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit2tag")){
        f = fopen("subreddit2tag.csv", "w");
        if (categories_wl_size == 0)
            compsky::mysql::query_buffer(&RES, "SELECT B.name, C.name FROM subreddit2tag A, subreddit B, tag C WHERE B.id=A.subreddit_id AND C.id=A.tag_id");
        else
            compsky::mysql::query(&RES, "SELECT DISTINCT s.name, t.name FROM subreddit2tag s2t, subreddit s, tag t, tag2category t2c, category c WHERE s.id=s2t.subreddit_id AND t.id=s2t.tag_id AND t2c.tag_id=t.id AND t2c.category_id=c.id AND c.name IN ('",  _f::start, "','", 3, categories_wl, categories_wl_size, _f::end, "')");
        // Use names rather than IDs to simplify importing between different databases
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &name2))
            // \t and \n are the two non-null characters that are impossible to include in a tag name when creating the tag names through the Qt GUI.
            compsky::asciify::write(f,  name, '\t', name2, '\n');
        fclose(f);
    }
    
    
    compsky::mysql::exit_mysql();
}
