#include <stdio.h> // for printf
#include <string.h> // for strlen, memcpy
#include <unistd.h> // for STDIN_FILENO, STDOUT_FILENO, read

/* MySQL */
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#ifdef DEBUG
  #define PRINTF(...) printf(__VA_ARGS__);
#else
  #define PRINTF(...) ;
#endif


int count_digits(unsigned long int n){
    // Obviously for non-negative integers only
    int count = n ? 0 : 1; // Need to have 1 digit in the special case n == 0
    while (n != 0){
        ++count;
        n /= 10;
    }
    //PRINTF("count_digits(i): %d\n", count);
    return count;
}

template<typename T>
void itoa_nonstandard(T n, const int n_digits, char* buf){
    // Obviously not safe etc. etc. No util functions are written for safety.
    // buf should be at least as long as n_digits+1
    int i = n_digits;
    while (--i >= 0){
        buf[i] = '0' + (n % 10);
        n /= 10;
    }
};

template<typename T>
int itoa_nonstandard(T n, char* buf){
    // Obviously not safe etc. etc. No util functions are written for safety.
    // buf should be at least as long as n_digits+1
    int n_digits = count_digits(n);
    itoa_nonstandard(n, n_digits, buf);
    return n_digits;
};

void read_fp_from_diraggr(char* dir, int& dir_len, char* fname, char* fp){
    int len;
    
    goto goto__readindfsd;
    
    while (len == 0){
        // Change of directory
        // Might have multiple changes of directory before a filename is listed
        read(STDIN_FILENO,  &dir_len,  sizeof(dir_len));
        read(STDIN_FILENO,  dir,  dir_len);
        memcpy(fp,  dir,  dir_len);
        fp[dir_len] = '/';
        
        goto__readindfsd:
        read(STDIN_FILENO,  &len,  sizeof(len));
    }
    
    read(STDIN_FILENO,  fname,  len);
    memcpy(fp + dir_len + 1,  fname,  len);
    fp[dir_len + 1 + len] = 0;
    fname[len] = 0;
}

void sql__insert_into_table_at(sql::Statement* sql_stmt, sql::ResultSet* sql_res, const char* table_name, const char* entry_name, const unsigned long int id){
    if (entry_name[0] == 0)
        return;
    
    int i;
    const char* a = "INSERT IGNORE INTO ";
    const char* b = " (id, name) values(";
    char stmt[strlen(a) + strlen(table_name) + strlen(b) + count_digits(id) + 2 + strlen(entry_name) + 3 + 1];
    
    i = 0;
    
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(stmt + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    
    memcpy(stmt + i,  b,  strlen(b));
    i += strlen(b);
    
    i += itoa_nonstandard(id,  stmt + i);
    
    stmt[i++] = ',';
    stmt[i++] = '"';
    
    memcpy(stmt + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    
    stmt[i++] = '"';
    stmt[i++] = ')';
    stmt[i++] = ';';
    stmt[i] = 0;
    
    PRINTF("stmt: %s\n", stmt);
    sql_stmt->execute(stmt);
    
    return;
}

int sql__get_id_from_table(sql::Statement* sql_stmt, sql::ResultSet* sql_res, const char* table_name, const char* entry_name){
    if (entry_name[0] == 0)
        return 0;
    
    int i;
    char statement[2048];
    
    goto__select_from_table:
    
    i = 0;
    
    const char* a = "SELECT id FROM ";
    memcpy(statement + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    
    const char* dummy = " WHERE name = \"";
    memcpy(statement + i,  dummy,  strlen(dummy));
    i += strlen(dummy);
    
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    
    statement[i++] = '"';
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    sql_res = sql_stmt->executeQuery(statement);
    
    if (sql_res->next()){
        // Entry already existed in table
        PRINTF("Result ID: %d\n", sql_res->getInt(1));
        return sql_res->getInt(1);
    }
    
    PRINTF("Creating new entry in '%s' for '%s'\n", table_name, entry_name);
    
    i = 0;
    const char* statement2 = "INSERT INTO ";
    memcpy(statement + i,  statement2,  strlen(statement2));
    i += strlen(statement2);
    
    memcpy(statement + i,  table_name,  strlen(table_name));
    i += strlen(table_name);
    
    const char* fff = " (name) values(\"";
    memcpy(statement + i,  fff,  strlen(fff));
    i += strlen(fff);
    
    memcpy(statement + i,  entry_name,  strlen(entry_name));
    i += strlen(entry_name);
    
    statement[i++] = '"';
    statement[i++] = ')';
    statement[i++] = ';';
    statement[i] = 0;
    
    PRINTF("stmt: %s\n", statement);
    sql_stmt->execute(statement);
    
    goto goto__select_from_table;
}

int sql__file_attr_id(sql::Statement* sql_stmt, sql::ResultSet* sql_res, const char* attr, int attr_id_int, const char* file_id, const int file_id_len){
    char stmt[1024];
    int i;
    
    
    goto__fileattridselect:
    i = 0;
    
    const char* a = "SELECT id FROM file2";
    memcpy(stmt + i,  a,  strlen(a));
    i += strlen(a);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* b = " WHERE (file_id, ";
    memcpy(stmt + i,  b,  strlen(b));
    i += strlen(b);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* c = "_id) = (\"";
    memcpy(stmt + i,  c,  strlen(c));
    i += strlen(c);
    
    memcpy(stmt + i,  file_id,  file_id_len);
    i += file_id_len;
    
    const char* d = "\", \"";
    memcpy(stmt + i,  d,  strlen(d));
    i += strlen(d);
    
    i += itoa_nonstandard(attr_id_int,  stmt + i);
    
    const char* e = "\");";
    memcpy(stmt + i,  e,  strlen(e));
    i += strlen(e);
    
    stmt[i] = 0;
    
    PRINTF("stmt: %s\n", stmt);
    sql_res = sql_stmt->executeQuery(stmt);
    
    if (sql_res->next())
        return sql_res->getInt(1); // 1 is first column
    else
        PRINTF("No prior tags of this value\n");
    
    
    i = 0;
    
    const char* f = "INSERT INTO file2";
    memcpy(stmt + i,  f,  strlen(f));
    i += strlen(f);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* g = " (file_id, ";
    memcpy(stmt + i,  g,  strlen(g));
    i += strlen(g);
    
    memcpy(stmt + i,  attr,  strlen(attr));
    i += strlen(attr);
    
    const char* h = "_id) values(\"";
    memcpy(stmt + i,  h,  strlen(h));
    i += strlen(h);
    
    memcpy(stmt + i,  file_id,  file_id_len);
    i += file_id_len;
    
    // ", "
    memcpy(stmt + i,  d,  strlen(d));
    i += strlen(d);
    
    i += itoa_nonstandard(attr_id_int,  stmt + i);
    
    memcpy(stmt + i,  e,  strlen(e));
    i += strlen(e);
    
    stmt[i] = 0;
    
    PRINTF("stmt: %s\n", stmt);
    sql_stmt->execute(stmt);
    
    goto goto__fileattridselect; // Return the table entry id
}
