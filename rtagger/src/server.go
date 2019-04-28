/*
 * Synopsis: [EXECUTABLE_PATH] [AUTHORISATION_FILE_PATH]   :[PORT]
 * Example:  build/server      C:/Users/me/reddit_auth.txt :8000
*/

package main

/*
#cgo LDFLAGS: ${PWD}/build/rtagger.so
// Relative path doesn't seem to work (-Lbuild -lrtagger)
#cgo CFLAGS: -O3

extern char* DST;
extern void init_mysql_from_file(const char* fp);
extern void csv2cls(const char* csv);
extern void free_dst();
*/
import "C" // Pseudopackage
import "io"
import "net/http"
import "os"


func process(w http.ResponseWriter, r* http.Request){
    C.csv2cls(C.CString(r.URL.Path))
    io.WriteString(w, C.GoString(C.DST))
    C.free_dst()
}

func main(){
    C.init_mysql_from_file(C.CString(os.Args[1]))
    mux := http.NewServeMux()
    mux.HandleFunc("/", process)
    http.ListenAndServe(os.Args[2], mux)
}
