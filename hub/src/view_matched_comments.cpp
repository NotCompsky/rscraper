/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "view_matched_comments.hpp"

#include <boost/regex.hpp>
#include <ctime> // for localtime, time_t

#include <QCompleter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QRadioButton>
#include <QVBoxLayout>

#define ASCIIFY_TIME
#include <compsky/asciify/asciify.hpp>
#include <compsky/mysql/query.hpp>

#include "init_regexp_from_file.hpp"

#include "id2str.hpp"


extern QStringList tagslist;
extern QCompleter* reason_name_completer;

namespace filter_comment_body {
    extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;
}

namespace _f {
    constexpr static const compsky::asciify::flag::ChangeBuffer chbuf;
}


constexpr const char* tag_a1 = 
    "SELECT S.name, S.id, c.id, c.created_at, c.content, u.name, m.name " // Dummy column '' to substitute for 'reason' column
    "FROM reason_matched m, user u, comment c "
    "JOIN ("
        "SELECT R.name, s.id "
        "FROM submission s "
        "JOIN ( "
            "SELECT r.id, r.name "
            "FROM subreddit r ";
constexpr const char* tag_b1 = 
            "JOIN ( "
                "SELECT s2t.subreddit_id "
                "FROM subreddit2tag s2t "
                "JOIN ("
                    "SELECT t.id "
                    "FROM tag t "
                    "WHERE t.name=\"";

constexpr const char* tag_b2 = 
                "\") T on T.id = s2t.tag_id "
            ") S2T on S2T.subreddit_id = r.id ";
constexpr const char* tag_a2 = 
        ") R on R.id = s.subreddit_id "
    ") S on S.id = c.submission_id "
    "WHERE u.id=c.author_id "
    "AND m.id=c.reason_matched";

constexpr const char* reason_a1 = 
    "SELECT r.name, s.id, c.id, c.created_at, c.content, u.name, m.name "
    "FROM subreddit r, submission s, comment c, user u, reason_matched m "
    "WHERE ";
constexpr const char* reason_b1 = 
    "m.name=\"";

constexpr const char* reason_b2 = 
    "\" AND ";
constexpr const char* reason_a2 = 
    "c.reason_matched=m.id "
    "AND s.id=c.submission_id "
    "AND r.id=s.subreddit_id "
    "AND u.id=c.author_id";


ViewMatchedComments::ViewMatchedComments(QWidget* parent) : QWidget(parent), res1(0), is_ascending(false), cmnt_body(nullptr) {
    QVBoxLayout* l = new QVBoxLayout;
    
    
    {
    QHBoxLayout* box = new QHBoxLayout;
    box->addWidget(new QLabel("Tag Name:", this));
    this->tagname_input = new QLineEdit(this);
    QCompleter* tagcompleter = new QCompleter(tagslist);
    this->tagname_input->setCompleter(tagcompleter);
    box->addWidget(this->tagname_input);
    l->addLayout(box);
    }
    
    
    {
    QHBoxLayout* box = new QHBoxLayout;
    box->addWidget(new QLabel("Reason Matched:", this));
    this->reasonname_input = new QLineEdit(this);
    this->reasonname_input->setCompleter(reason_name_completer);
    box->addWidget(this->reasonname_input);
    l->addLayout(box);
    }
    
    
    {
    QGroupBox* group_box = new QGroupBox("Order By Date:");
    QRadioButton* asc   = new QRadioButton("Ascending");
    QRadioButton* desc  = new QRadioButton("Descending");
    connect(asc,  &QRadioButton::toggled, this, &ViewMatchedComments::toggle_order_btns);
    desc->setChecked(true);
    QHBoxLayout* box = new QHBoxLayout;
    box->addWidget(asc);
    box->addWidget(desc);
    box->addStretch(1);
    group_box->setLayout(box);
    l->addWidget(group_box);
    }
    
    
    {
    QHBoxLayout* box = new QHBoxLayout;
    QPushButton* next_btn = new QPushButton("Query", this);
    box->addWidget(next_btn);
    connect(next_btn, &QPushButton::clicked, this, &ViewMatchedComments::init);
    
    QPushButton* init_btn = new QPushButton("Next", this);
    box->addWidget(init_btn);
    connect(init_btn, &QPushButton::clicked, this, &ViewMatchedComments::next);
    
    l->addLayout(box);
    }
    
    
    this->subname   = new QLabel(this);
    this->username  = new QLabel(this);
    this->reasonname = new QLabel(this);
    this->datetime  = new QLabel(this);
    this->permalink = new QLineEdit(this);
    this->permalink->setReadOnly(true);
    l->addWidget(this->subname);
    l->addWidget(this->username);
    l->addWidget(this->reasonname);
    l->addWidget(this->datetime);
    l->addWidget(this->permalink);
    
    this->textarea = new QPlainTextEdit(this);
    this->textarea->setReadOnly(true);
    
    l->addWidget(this->textarea);
    
    QPushButton* details_cmnt = new QPushButton("Details", this);
    connect(details_cmnt, &QPushButton::clicked, this, &ViewMatchedComments::view_matches);
    l->addWidget(details_cmnt);
    
    QPushButton* del_cmnt = new QPushButton("Delete", this);
    connect(del_cmnt, &QPushButton::clicked, this, &ViewMatchedComments::del_cmnt);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Button, QColor(Qt::red));
    del_cmnt->setAutoFillBackground(true);
    del_cmnt->setPalette(palette);
    del_cmnt->setFlat(true);
    del_cmnt->update();
    l->addWidget(del_cmnt);
    
    this->setLayout(l);
}

ViewMatchedComments::~ViewMatchedComments(){
    delete this->textarea;
    delete this->datetime;
    delete this->reasonname;
    delete this->username;
    delete this->subname;
}

void ViewMatchedComments::init(){
    if (this->res1 != nullptr)
        mysql_free_result(this->res1);
    
    const QString tag    = this->tagname_input->text();
    const QString reason = this->reasonname_input->text();
    
    compsky::asciify::asciify(_f::chbuf, compsky::asciify::BUF, 0);
    
    if (!tag.isEmpty())
        if (!reason.isEmpty())
            // TODO: Make different
            compsky::asciify::asciify(tag_a1, tag_b1, tag, tag_b2, tag_a2);
        else
            compsky::asciify::asciify(tag_a1, tag_b1, tag, tag_b2, tag_a2);
    else if (!reason.isEmpty())
        compsky::asciify::asciify(reason_a1, reason_b1, reason, reason_b2, reason_a2);
    else
        compsky::asciify::asciify(reason_a1, reason_a2);
    
    compsky::asciify::asciify(" ORDER BY c.created_at ");
    compsky::asciify::asciify((this->is_ascending) ? "asc" : "desc");
    
    compsky::mysql::query_buffer(&this->res1, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
    
    this->next();
}

void ViewMatchedComments::next(){
    if (this->res1 == nullptr){
        this->subname->setText("");
        this->username->setText("");
        this->reasonname->setText("");
        this->datetime->setText("");
        this->textarea->setPlainText("");
        return;
    }
    char* subname;
    constexpr static const compsky::asciify::flag::StrLen f;
    uint64_t post_id;
    uint64_t t;
    char* username;
    char* reason;
    if (compsky::mysql::assign_next_row(this->res1, &this->row1, &subname, &post_id, &this->cmnt_id, &t, f, &this->cmnt_body_sz, &this->cmnt_body, &username, &reason)){
        post_id_str[id2str(post_id,         post_id_str)] = 0;
        cmnt_id_str[id2str(this->cmnt_id,   cmnt_id_str)] = 0;
        
        this->permalink->setText(QString("https://www.reddit.com/r/" + QString(subname) + QString("/comments/") + QString(post_id_str) + QString("/_/") + QString(cmnt_id_str)));
        
        const time_t tt = t;
        const struct tm* dt = localtime(&tt);
        char* dt_buf = compsky::asciify::BUF;
        compsky::asciify::asciify(_f::chbuf, dt_buf, 0,  dt);
        compsky::asciify::BUF[compsky::asciify::BUF_INDX] = 0;
        
        this->subname->setText(subname);
        this->username->setText(username);
        this->reasonname->setText(reason);
        this->datetime->setText(dt_buf);
        
        this->textarea->setPlainText(cmnt_body);
    } else this->res1 = nullptr;
}

void ViewMatchedComments::toggle_order_btns(){
    this->is_ascending = !this->is_ascending;
}

void ViewMatchedComments::del_cmnt(){
    compsky::mysql::exec("DELETE FROM comment WHERE id=", this->cmnt_id);
    this->next();
}

void ViewMatchedComments::view_matches(){
    /*
     * NOTE: The current regexp is run on the string, notably not necessarily the one that originally matched the comment.
     */
    
    std::vector<char*> reason_name2id;
    std::vector<int> groupindx2reason;
    std::vector<bool> record_contents;
    
    QString report = ""; 
    
    if (filter_comment_body::regexpr == nullptr)
        filter_comment_body::init_regexp_from_file(reason_name2id, groupindx2reason, record_contents);
    
    if (filter_comment_body::regexpr == nullptr){
        report += "Cannot find regexpr file. Ensure that the environmental variable RSCRAPER_REGEX_FILE is set to the file path of the regex file.";
    }
    
    boost::match_results<const char*> what;
    
    const char* str = this->cmnt_body;
    
    if (!boost::regex_search(str,  str + this->cmnt_body_sz,  what,  *filter_comment_body::regexpr))
        report += "No matches";
    
    for (auto i = 1;  i < what.size();  ++i){
        // Ignore first index - it is the entire match, not a regex group.
        if (what[i].matched)
            report += QString("\nMatched group ") + QString::number(i) + QString("\n\t") + QString::fromLocal8Bit(what[i].first,  (uintptr_t)what[i].second - (uintptr_t)what[i].first);
    }
    
    QMessageBox::information(this, "Report", report);
}
