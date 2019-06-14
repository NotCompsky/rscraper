#include <stdio.h> // for fwrite
#include <iostream> // for std::cout

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
    char name1[1024];
    char name2[1024];
    
    char buf[1024];
    
    ++argv;
    --argc;
    
    
    if (argc == 0  ||  contains(argv, argc, "user.csv")){
        f = fopen("user.csv", "r");
        while(fscanf(f, "%lu\t%s", &id, name1) != EOF)
            std::cout << id << "[u]: " << name1 << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit.csv")){
        f = fopen("subreddit.csv", "r");
        while(fscanf(f, "%lu\t%s", &id, name1) != EOF)
            std::cout << id << "[r]: " << name1 << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "subreddit2tag.csv")){
        f = fopen("subreddit2tag.csv", "r");
        while(fscanf(f, "%s\t%s", name1, name2) != EOF)
            std::cout << name1 << "[s2t]: " << name2 << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag.csv")){
        double r, g, b, a;
        f = fopen("tag.csv", "r");
        while(fscanf(f, "%s\t%lf\t%lf\t%lf\t%lf", name1, &r, &g, &b, &a) != EOF)
            std::cout << name1 << "[s2t]: " << r << "," << g << "," << b << "," << a << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "user2subreddit_cmnt_count.csv")){
        f = fopen("user2subreddit_cmnt_count.csv", "r");
        while(fscanf(f, "%lu\t%lu\t%lu", &user_id, &subreddit_id, &count) != EOF)
            std::cout << user_id << "[u2scc]: " << subreddit_id << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "tag2category.csv")){
        f = fopen("tag2category.csv", "r");
        while(fscanf(f, "%s\t%s", name1, name2) != EOF)
            std::cout << name1 << "[t2c]: " << name2 << std::endl;
        fclose(f);
    }
    
    if (argc == 0  ||  contains(argv, argc, "category.csv")){
        f = fopen("category.csv", "r");
        while(fscanf(f, "%s", name1) != EOF)
            std::cout << name1 << "[c]" << std::endl;
        fclose(f);
    }
    
    compsky::mysql::exit();
}
