#include <stdio.h> // for fwrite

#include <compsky/mysql/query.hpp>
#include <compsky/asciify/print.hpp>


MYSQL_RES* RES;
MYSQL_ROW ROW;

namespace compsky {
    namespace asciify {
        char* BUF = (char*)malloc(4096);
        constexpr static const size_t BUF_SZ = 4096;
            
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
    
    
    if (argc == 0  ||  contains(argv, argc, "user")){
        f = fopen("user.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT id, name FROM user");
        while(compsky::mysql::assign_next_row(RES, &ROW, &id, &name))
            compsky::asciify::write(f,  id, ',', name, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit")){
        f = fopen("subreddit.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT id, name FROM subreddit");
        while(compsky::mysql::assign_next_row(RES, &ROW, &id, &name))
            compsky::asciify::write(f,  id, ',', name, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit2tag")){
        f = fopen("subreddit2tag.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT B.name, C.name FROM subreddit2tag A, subreddit B, tag C WHERE B.id=A.subreddit_id AND C.id=A.tag_id");
        // Use names rather than IDs to simplify importing between different databases
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &name2))
            // \0 and \n are the two characters that are impossible to include in a tag name when creating the tag names through the Qt GUI.
            compsky::asciify::write(f,  name, '\0', name2, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag")){
        double r, g, b, a;
        f = fopen("tag.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT name r, g, b, a FROM tag");
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &r, &g, &b, &a))
            compsky::asciify::write(f,  _f::start, ',', name, r, g, b, a, _f::end,  '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "user2subreddit_cmnt_count")){
        f = fopen("user2subreddit_cmnt_count.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT user_id, subreddit_id, count FROM user2subreddit_cmnt_count");
        while(compsky::mysql::assign_next_row(RES, &ROW, &user_id, &subreddit_id, &count))
            compsky::asciify::write(f,  user_id, ',', subreddit_id, ',', count, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag2category")){
        f = fopen("tag2category.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT B.name, C.name FROM tag2category A, tag B, category C WHERE B.id=A.tag_id AND C.id=A.category_id");
        while(compsky::mysql::assign_next_row(RES, &ROW, &name, &name2))
            compsky::asciify::write(f,  name, '\0', name2, '\n');
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "category")){
        f = fopen("category.csv", "w");
        compsky::mysql::query_buffer(&RES, "SELECT name FROM category");
        while(compsky::mysql::assign_next_row(RES, &ROW, &name))
            compsky::asciify::write(f,  name, '\n');
        fclose(f);
    }
    
    compsky::mysql::exit();
}
