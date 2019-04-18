#include <rapidjson/document.h> // for rapidjson::Document
#include "rapidjson/pointer.h" // for rapidjson::GetValueByPointer
#include <stdio.h> // for fread, printf

#define REDDIT_REQUEST_DELAY 1

#define SET_DBG_STR(a, b) const char* a = b; printf(#a ":\t%s\n", b);
#define SET_DBG_INT(a, b) const int a = b; printf(#a ":\t%d\n", b);


void printf_indent(const int n){
    for (auto i = 0;  i < n;  ++i)
        printf("\t");
}

void print_all_json_values(rapidjson::Value& val, rapidjson::Value::AllocatorType& allocator, int depth){
    if (val.IsObject())
        for (rapidjson::Value::MemberIterator itr = val.MemberBegin();  itr != val.MemberEnd();  ++itr)
            print_all_json_values(itr->value, allocator, depth+1);
    else if (val.IsArray())
        for (rapidjson::Value::ValueIterator itr = val.Begin();  itr != val.End();  ++itr)
            print_all_json_values(*itr, allocator, depth+1);
    else if (val.IsString()){
        printf_indent(depth);
        printf("%s\n", val.GetString());
    }
    else if (val.IsInt()){
        printf_indent(depth);
        printf("%d\n", val.GetInt());
    }
}


int main(const int argc, const char* argv[]){
    FILE* f = fopen("/tmp/b", "r");
    char buf[19980];
    fread(buf, 1, 19980, f);
    rapidjson::Document d;
    if (d.Parse(buf).HasParseError())
        return 1;
    
    SET_DBG_STR(title, d[0]["data"]["children"][0]["data"]["title"].GetString())
    
    SET_DBG_STR(author_id, d[0]["data"]["children"][0]["data"]["author_fullname"].GetString()); // t2_<ID>
    SET_DBG_STR(author, d[0]["data"]["children"][0]["data"]["author"].GetString());
    SET_DBG_INT(score, d[0]["data"]["children"][0]["data"]["score"].GetInt());
   // const time_t created_at = d[0]["data"]["children"][0]["data"]["created"].GetInt64();
    SET_DBG_STR(link_domain, d[0]["data"]["children"][0]["data"]["domain"].GetString());
    SET_DBG_STR(link_url, d[0]["data"]["children"][0]["data"]["url"].GetString());
    
    print_all_json_values(d, d.GetAllocator(), 0);
}
