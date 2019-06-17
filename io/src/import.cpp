#include <stdio.h> // for fwrite
#include <stdlib.h> // for malloc
#include <iostream> // for std::cout

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/print.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;


constexpr size_t strlen_constexpr(const char* s){
    // GCC strlen is constexpr; this is apparently a bug
    return *s  ?  1 + strlen_constexpr(s + 1)  :  0;
}


namespace compsky {
    namespace asciify {
        constexpr static const size_t BUF_SZ = 4 * 1024 * 1024;
        char* BUF = (char*)malloc(4 * 1024 * 1024); // 4 MiB
            
        void ensure_buf_can_fit(const char* pre,  size_t n){
            if (unlikely(BUF_INDX + n  >  BUF_SZ)){
                compsky::mysql::exec_buffer(BUF,  BUF_INDX - 1); // Ignore trailing comma
                BUF_INDX = 0;
                asciify(pre);
            }
        }
        
        void ensure_buf_can_fit(const char* pre,  size_t n,  const char* post){
            if (unlikely(BUF_INDX + n + strlen(post)  >  BUF_SZ)){
                --BUF_INDX; // Overwrite trailing comma
                memcpy(BUF + BUF_INDX,  post,  strlen(post));
                BUF_INDX += strlen(post);
                compsky::mysql::exec_buffer(BUF,  BUF_INDX - 1);
                BUF_INDX = 0;
                asciify(pre);
            }
        }
    }
}

namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}

bool contains(const char** ls,  const int n,    const char* s){
    const size_t slen = strlen_constexpr(s);
    for (auto i = 0;  i < n;  ++i)
        if (strncmp(ls[i], s, slen) == 0)
            return true;
    return false;
}

int main(int argc,  const char** argv){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));  // Init SQL
    
    FILE* f;
    uint64_t count, id, subreddit_id, tag_id, user_id, category_id;
    char name1[1024];
    char name2[1024];
    
    char buf[1024];
    
    ++argv;
    --argc;
    
    
    // Arg Parser
    bool force = false;
    while(true){
        const char* s = *argv;
        if (s[2] != 0  ||  s[0] != '-')
            break;
        switch(s[1]){
            case 'f':
                force = true;
                break;
            default:
                fprintf(stderr, "Unrecognised option: %s\n", s);
                exit(1);
        }
        ++argv;
        --argc;
    }
    
    
    /* No dependencies on other tables */
    
    if (argc == 0  ||  contains(argv, argc, "user.csv")){
        f = fopen("user.csv", "r");
        compsky::asciify::BUF_INDX = 0;
        constexpr static const char* pre = "INSERT IGNORE INTO user (id,name) VALUES ";
        compsky::asciify::asciify(pre);
        while(fscanf(f, "%lu\t%s", &id, name1) != EOF){
            compsky::asciify::asciify('(', id, ',', '"', _f::esc, '"', name1, '"', ')', ',');
            compsky::asciify::ensure_buf_can_fit(pre,  1 + 19 + 1 + 1 + 1 + 2*128 + 1 + 1 + 1);
        }
        if (compsky::asciify::BUF_INDX != strlen_constexpr(pre))
            compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Overwrite trailing comma
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit.csv")){
        f = fopen("subreddit.csv", "r");
        compsky::asciify::BUF_INDX = 0;
        constexpr static const char* pre = "INSERT IGNORE INTO subreddit (id,name) VALUES ";
        compsky::asciify::asciify(pre);
        while(fscanf(f, "%lu\t%s", &id, name1) != EOF){
            compsky::asciify::asciify('(', id, ',', '"', _f::esc, '"', name1, '"', ')', ',');
            compsky::asciify::ensure_buf_can_fit(pre,  1 + 19 + 1 + 1 + 1 + 2*128 + 1 + 1 + 1);
        }
        if (compsky::asciify::BUF_INDX != strlen_constexpr(pre))
            compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Overwrite trailing comma
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag.csv")){
        double r, g, b, a;
        f = fopen("tag.csv", "r");
        compsky::asciify::BUF_INDX = 0;
        constexpr static const char* pre = "INSERT IGNORE INTO tag (name, r, g, b, a) VALUES ";
        compsky::asciify::asciify(pre);
        while(fscanf(f, "%s\t%s", name1, name2) != EOF){
            // `name2` itself is of the format %lf,%lf,%lf,%lf
            compsky::asciify::asciify("(\"", _f::esc, '"', name1, "\",", name2, "),");
            compsky::asciify::ensure_buf_can_fit(pre,  2 + 2*128 + 2 + 4*7 + 2);
        }
        if (compsky::asciify::BUF_INDX != strlen_constexpr(pre))
            compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Overwrite trailing comma
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "category.csv")){
        double r, g, b, a;
        f = fopen("category.csv", "r");
        compsky::asciify::BUF_INDX = 0;
        constexpr static const char* pre = "INSERT IGNORE INTO category (name) VALUES ";
        compsky::asciify::asciify(pre);
        while(fscanf(f, "%s\n", name1) != EOF){
            compsky::asciify::asciify("(\"", _f::esc, '"', name1, "\"),");
            compsky::asciify::ensure_buf_can_fit(pre,  2 + 2*128 + 3);
        }
        if (compsky::asciify::BUF_INDX != strlen_constexpr(pre))
            compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Overwrite trailing comma
        fclose(f);
    }
    
    /* No dependencies on previous import data (i.e. just using absolute IDs) */
    
    if (argc == 0  ||  contains(argv, argc, "user2subreddit_cmnt_count.csv")){
        f = fopen("user2subreddit_cmnt_count.csv", "r");
        compsky::asciify::BUF_INDX = 0;
        char* pre;
        char* post;
        if (force){
            pre  = "INSERT INTO user2subreddit_cmnt_count (user_id,subreddit_id,count) VALUES ";
            post = " ON DUPLICATE KEY SET count=VALUES(count)";
        } else {
            pre = "INSERT IGNORE INTO user2subreddit_cmnt_count (user_id,subreddit_id,count) VALUES ";
            post = " ";
        }
        compsky::asciify::asciify(pre);
        while(fscanf(f, "%lu\t%lu\t%lu", &user_id, &subreddit_id, &count) != EOF){
            compsky::asciify::asciify("(\"", _f::esc, '"', name1, "\"),");
            compsky::asciify::ensure_buf_can_fit(pre,  2 + 2*128 + 3,  post);
        }
        if (compsky::asciify::BUF_INDX != strlen(pre))
            compsky::mysql::exec_buffer(compsky::asciify::BUF,  compsky::asciify::BUF_INDX - 1); // Overwrite trailing comma
        fclose(f);
    }
    
    /* Name-to-name tables */
    
    if (argc == 0  ||  contains(argv, argc, "subreddit2tag.csv")){
        f = fopen("subreddit2tag.csv", "r");
        while(fscanf(f, "%s\t%s", name1, name2) != EOF){
            compsky::mysql::exec("INSERT IGNORE INTO subreddit2tag (subreddit_id,tag_id) SELECT s.id,t.id FROM subreddit s, tag t WHERE s.name=\"", _f::esc, '"', name1, "\" AND t.name=\"", _f::esc, '"', name2, "\"");
        }
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag2category.csv")){
        f = fopen("tag2category.csv", "r");
        while(fscanf(f, "%s\t%s", name1, name2) != EOF)
            compsky::mysql::exec("INSERT IGNORE INTO tag2category (tag_id,category_id) SELECT t.id,c.id FROM tag t, category c WHERE t.name=\"", _f::esc, '"', name1, "\" AND c.name=\"", _f::esc, '"', name2, "\"");
        fclose(f);
    }
    
    
    compsky::mysql::exit_mysql();
}
