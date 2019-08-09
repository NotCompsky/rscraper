package main

/*
#cgo LDFLAGS: -L${SRCDIR}/../../build/tagger -lrscraper-tagger
#cgo CFLAGS: -O3

extern char* DST;
extern void init();
extern void exit_mysql();
extern const char* generate_id_list_string(const char* tblname,  const char** names);
extern void csv2cls(const char* csv,  const char* tagcondition,  const char* reasoncondition);
extern void user_summary(const char* reasonfilter,  const char* const name);
extern void comments_given_reason(const char* const reasonfilter,  const char* const reason_name);
extern void subreddits_given_reason(const char* const reasonfilter,  const char* const reason_name);
*/
import "C" // Pseudopackage
import "flag"
import "io"
import "net/http"
import "os"
import "syscall"
import "os/signal"


var tagfilter string
var reasonfilter string


func process(w http.ResponseWriter, r* http.Request){
    w.Header().Set("Content-Type", "application/json")
    C.csv2cls(C.CString(r.URL.Path[8:]), C.CString(tagfilter), C.CString(reasonfilter))
    io.WriteString(w, C.GoString(C.DST))
}

func process_user(w http.ResponseWriter, r* http.Request){
    C.user_summary(C.CString(reasonfilter), C.CString(r.URL.Path[3:]))
    io.WriteString(w, C.GoString(C.DST))
}

func subreddits_given_reason(w http.ResponseWriter, r* http.Request){
    C.subreddits_given_reason(C.CString(reasonfilter), C.CString(r.URL.Path[19:]))
    io.WriteString(w, C.GoString(C.DST))
}

func comments_given_reason(w http.ResponseWriter, r* http.Request){
    C.comments_given_reason(C.CString(reasonfilter), C.CString(r.URL.Path[17:]))
    io.WriteString(w, C.GoString(C.DST))
}

func indexof_reason(w http.ResponseWriter, r* http.Request){
	const HTML_indexof_reason = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<h1>" +
					"Statistics or links for a given reason" +
				"</h1>" +
				"<a href=\"subreddits\">" +
					"Subreddits" +
				"</a>" +
				"<br/>" +
				"<a href=\"comments\">" +
					"Comments" +
				"</a>" +
			"</body>" +
		"</html>"
    io.WriteString(w, HTML_indexof_reason)
}

func indexof_root(w http.ResponseWriter, r* http.Request){
	const HTML_indexof_root = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<h1>" +
					"Index" +
				"</h1>" +
				"<a href=\"reason\">" +
					"Reason Statistics" +
				"</a>" +
				"<br/>" +
				"<a href=\"u\">" +
					"User Statistics" +
				"</a>" +
			"</body>" +
		"</html>"
    io.WriteString(w, HTML_indexof_root)
}

func main(){
    var portN string
    flag.StringVar(&portN, "p", "8080", "Port number")
    flag.StringVar(&tagfilter,    "t", "", "SQL condition that t.id (tag ID) must fulfil. If non-empty, must begin with 'AND'. E.g. 'AND t.id=3'")
    flag.StringVar(&reasonfilter, "m", "", "SQL condition that m.id (reason_matched ID) must fulfil. If non-empty, must begin with 'AND'. E.g. 'AND m.id=3'")
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
	
	mux.HandleFunc("/", indexof_root)
	mux.HandleFunc("/reason/", indexof_reason)
	
    mux.HandleFunc("/flairs/", process)
	
	mux.HandleFunc("/reason/subreddits/", subreddits_given_reason)
	mux.HandleFunc("/reason/comments/",   comments_given_reason)
	
	mux.HandleFunc("/u/", process_user)
	
    http.ListenAndServe(":" + portN,  mux)
}

/*
ISSUES:
    Exits normally (returns 0) immediately if the port is unusable - either bad format (should be colon preceding number, e.g. :12345) or already in use.
*/
