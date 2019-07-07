package main

/*
#cgo LDFLAGS: -L${SRCDIR}/../../build/tagger -lrscraper-tagger
#cgo CFLAGS: -O3

extern char* DST;
extern void init();
extern void exit_mysql();
extern void csv2cls(const char* csv);
*/
import "C" // Pseudopackage
import "flag"
import "io"
import "net/http"
import "os"
import "syscall"
import "os/signal"


func process(w http.ResponseWriter, r* http.Request){
    w.Header().Set("Content-Type", "application/json")
    C.csv2cls(C.CString(r.URL.Path))
    io.WriteString(w, C.GoString(C.DST))
}

func main(){
    var portN string
    flag.StringVar(&portN, "p", "8080", "Port number")
    flag.Parse()
    
    /* Exit MySQL on interrupt signals */
    sgnl := make(chan os.Signal)
    signal.Notify(sgnl, os.Interrupt, syscall.SIGTERM)
    go func(){
        <-sgnl
        C.exit_mysql()
        os.Exit(1)
    }()
    
    C.init()
    mux := http.NewServeMux()
    mux.HandleFunc("/", process)
    http.ListenAndServe(":" + portN,  mux)
}

/*
ISSUES:
    Exits normally (returns 0) immediately if the port is unusable - either bad format (should be colon preceding number, e.g. :12345) or already in use.
*/
