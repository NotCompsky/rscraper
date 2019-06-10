/*
 * Synopsis: [EXECUTABLE_PATH] [AUTHORISATION_FILE_PATH]   :[PORT]
 * Example:  build/server      C:/Users/me/reddit_auth.txt :8000
*/

package main

/*
#cgo LDFLAGS: -lrscraper-tagger
// Relative path doesn't seem to work (-Lbuild -lrtagger), so must be either absolute path to SO file, or the usual library link after the library has been installed to a system library directory. If the latter, the library must reside in $LD_LIBRARY_PATH (or the Windows equivalent).
#cgo CFLAGS: -O3

extern char* DST;
extern void init();
extern void csv2cls(const char* csv);
*/
import "C" // Pseudopackage
import "io"
import "net/http"
import "os"


func process(w http.ResponseWriter, r* http.Request){
    C.csv2cls(C.CString(r.URL.Path))
    io.WriteString(w, C.GoString(C.DST))
}

func main(){
    C.init()
    mux := http.NewServeMux()
    mux.HandleFunc("/", process)
    http.ListenAndServe(os.Args[1], mux)
}

/*
ISSUES:
    Exits normally (returns 0) immediately if the port is unusable - either bad format (should be colon preceding number, e.g. :12345) or already in use.
*/
