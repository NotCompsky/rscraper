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
extern void get_all_reasons(const char* const reasonfilter);
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

// NOTE: To convert JS/HTML to human readable format,  ^(\t*)"|" \+$|\\(")  ->  \1\2


func js_utils(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"function timestamp2dt(t){" +
			"return new Date(t*1000).toISOString().slice(-24, -5)" +
		"}" +
		"function wipe_table(selector){" +
			"$(selector + \" tr\").remove();" +
		"}" +
		"function populate_table(url, selector, postfnct){" +
			"$.ajax({" +
				"dataType: \"json\"," +
				"url: url," +
				"success: function(data){" +
					"var s = \"\";" +
					"for (var row of data){" +
						"s += \"<tr>\";" +
						"for (var item of row){" +
							"s += \"<td>\" + item + \"</td>\";" +
						"}" +
						"s += \"</tr>\";" +
					"}" +
					"$(selector).html(s);" +
					"if (postfnct !== undefined){" +
						"postfnct();" +
					"}" +
				"}," +
				"error: function(){" +
					"alert(\"Error populating table\");" +
				"}" +
			"});" +
		"}" +
		"function column_to_permalink(selector, subreddit_indx, link_indx){" +
			"$(selector).find('tr').each(function (i, el){" +
				"var $tds = $(this).find('td');" +
				"var subreddit_name = $tds.eq(subreddit_indx).text();" +
				"var $link = $tds.eq(link_indx);" +
				"$link.html(\"<a href='https://www.reddit.com/r/\" + subreddit_name + \"/comments/\" + $link.text() + \"'>Link</a>\")" +
			"});" +
		"}" +
		"function column_from_timestamp(selector, timestamp_indx){" +
			"$(selector).find('tr').each(function (i, el){" +
				"var $tds = $(this).find('td');" +
				"var $link = $tds.eq(timestamp_indx);" +
				"$link.value = $link.text();" +
				"$link.text(timestamp2dt($link.value));" +
			"});" +
		"}" +
		"function populate_reasons(){" +
			"$.ajax({" +
				"dataType: \"json\"," +
				"url: \"/api/reasons.json\"," +
				"success: function(data){" +
					"var s = \"\";" +
					"for (var name of data){" +
						"s += \"<option value='\" + name + \"'>\" + name + \"</option>\";" +
					"}" +
					"$(\"#m\").html(s);" +
				"}," +
				"error: function(){" +
					"alert(\"Error populating table\");" +
				"}" +
			"});" +
		"}" +
		"window.onload=populate_reasons;"
    io.WriteString(w, html)
}


func get_all_reasons(w http.ResponseWriter, r* http.Request){
    w.Header().Set("Content-Type", "application/json")
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
    C.get_all_reasons(C.CString(reasonfilter))
    io.WriteString(w, C.GoString(C.DST))
}


func indexof_flairs_given_users(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<h1>" +
					"RTagger Flairs" +
				"</h1>" +
				"<h2>" +
					"How to use" +
				"</h2>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
}

func flairs_given_users(w http.ResponseWriter, r* http.Request){
    w.Header().Set("Content-Type", "application/json")
    C.csv2cls(C.CString(r.URL.Path[12:]), C.CString(tagfilter), C.CString(reasonfilter))
    io.WriteString(w, C.GoString(C.DST))
}


func html_comments_given_user(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<script src=\"https://code.jquery.com/jquery-3.4.1.min.js\"></script>" +
				"<script src=\"/static/utils.js\"></script>" +
				"<script>" +
					"function format_table(){" +
						"column_to_permalink('#tbl tbody',  1,  3);" +
						"column_from_timestamp('#tbl tbody',  2);" +
					"}" +
				"</script>" +
				"<h1>" +
					"Comments given user" +
				"</h1>" +
				"<div>" +
					"<input type=\"text\" id=\"u\" placeholder=\"Username\"/>" +
					"<button onclick=\"wipe_table('#tbl tbody'); populate_table('/api/u/' + $('#u')[0].value,  '#tbl tbody',  format_table)\">" +
						"Go" +
					"</button>" +
				"</div>" +
				"<table id=\"tbl\">" +
					"<thead>" +
						"<tr>" +
							"<th>" +
								"Reason" +
							"</th>" +
							"<th>" +
								"Subreddit" +
							"</th>" +
							"<th>" +
								"At" +
							"</th>" +
							"<th>" +
								"Link" +
							"</th>" +
						"</tr>" +
					"</thead>" +
					"<tbody></tbody>" +
				"</table>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
}

func comments_given_user(w http.ResponseWriter, r* http.Request){
    C.user_summary(C.CString(reasonfilter), C.CString(r.URL.Path[7:]))
    io.WriteString(w, C.GoString(C.DST))
}


func html_subreddits_given_reason(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<script src=\"https://code.jquery.com/jquery-3.4.1.min.js\"></script>" +
				"<script src=\"/static/utils.js\"></script>" +
				"<h1>" +
					"Subreddits given reason" +
				"</h1>" +
				"<div>" +
					"<select id=\"m\"></select>" +
					"<button onclick=\"wipe_table('#tbl tbody'); populate_table('/api/reason/subreddits/' + $('#m')[0].value,  '#tbl tbody')\">" +
						"Go" +
					"</button>" +
				"</div>" +
				"<table id=\"tbl\">" +
					"<thead>" +
						"<tr>" +
							"<th>" +
								"Subreddit" +
							"</th>" +
							"<th>" +
								"Proportion" +
							"</th>" +
						"</tr>" +
					"</thead>" +
					"<tbody></tbody>" +
				"</table>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
}

func subreddits_given_reason(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
    C.subreddits_given_reason(C.CString(reasonfilter), C.CString(r.URL.Path[23:]))
    io.WriteString(w, C.GoString(C.DST))
}


func html_comments_given_reason(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<script src=\"https://code.jquery.com/jquery-3.4.1.min.js\"></script>" +
				"<script src=\"/static/utils.js\"></script>" +
				"<script>" +
					"function format_table(){" +
						"column_to_permalink('#tbl tbody',  0,  2);" +
						"column_from_timestamp('#tbl tbody',  1);" +
					"}" +
				"</script>" +
				"<h1>" +
					"Comments given reason" +
				"</h1>" +
				"<div>" +
					"<select id=\"m\"></select>" +
					"<button onclick=\"wipe_table('#tbl tbody'); populate_table('/api/reason/comments/' + $('#m')[0].value,  '#tbl tbody',  format_table);  \">" +
						"Go" +
					"</button>" +
				"</div>" +
				"<table id=\"tbl\">" +
					"<thead>" +
						"<tr>" +
							"<th>" +
								"Subreddit" +
							"</th>" +
							"<th>" +
								"At" +
							"</th>" +
							"<th>" +
								"Link" +
							"</th>" +
						"</tr>" +
					"</thead>" +
					"<tbody></tbody>" +
				"</table>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
}

func comments_given_reason(w http.ResponseWriter, r* http.Request){
    C.comments_given_reason(C.CString(reasonfilter), C.CString(r.URL.Path[21:]))
    io.WriteString(w, C.GoString(C.DST))
}


func indexof_reason(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<h1>" +
					"Statistics or links for a given reason" +
				"</h1>" +
				"<a href=\"subreddits/\">" +
					"Subreddits" +
				"</a>" +
				"<br/>" +
				"<a href=\"comments/\">" +
					"Comments" +
				"</a>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
}

func indexof_root(w http.ResponseWriter, r* http.Request){
	w.Header().Set("Cache-Control", "max-age=86400") // 24h
	const html = "" +
		"<!DOCTYPE html>" +
			"<body>" +
				"<h1>" +
					"Index" +
				"</h1>" +
				"<a href=\"flairs/\">" +
					"User Flairing" +
				"</a>" +
				"<br/>" +
				"<a href=\"reason/\">" +
					"Reason Statistics" +
				"</a>" +
				"<br/>" +
				"<a href=\"u/\">" +
					"User Statistics" +
				"</a>" +
			"</body>" +
		"</html>"
    io.WriteString(w, html)
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
	
	/* NOTE: 
		API is unstable.
		When it is stable, the following (/js and /api) may be versioned.
	*/
	
	mux.HandleFunc("/", indexof_root)
	mux.HandleFunc("/reason/", indexof_reason)
	
	mux.HandleFunc("/flairs/", indexof_flairs_given_users)
    mux.HandleFunc("/api/flairs/", flairs_given_users)
	
	mux.HandleFunc("/static/utils.js", js_utils)
	
	mux.HandleFunc("/reason/subreddits/", html_subreddits_given_reason)
	//mux.HandleFunc("/static/subreddits_given_reason.js", js_subreddits_given_reason)
	mux.HandleFunc("/api/reason/subreddits/", subreddits_given_reason)
	mux.HandleFunc("/reason/comments/",   html_comments_given_reason)
	//mux.HandleFunc("/static/comments_given_reason.js",   js_comments_given_reason)
	mux.HandleFunc("/api/reason/comments/",   comments_given_reason)
	
	mux.HandleFunc("/u/", html_comments_given_user)
	//mux.HandleFunc("/static/comments_given_user.js", js_comments_given_user)
	mux.HandleFunc("/api/u/", comments_given_user)
	
	mux.HandleFunc("/api/reasons.json",  get_all_reasons)
	
    http.ListenAndServe(":" + portN,  mux)
}

/*
ISSUES:
    Exits normally (returns 0) immediately if the port is unusable - either bad format (should be colon preceding number, e.g. :12345) or already in use.
*/
