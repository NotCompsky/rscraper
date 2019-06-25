/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "view_matched_comments.hpp"

#include <ctime> // for strftime, localtime, time_t

#include <QCompleter>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

#include "id2str.hpp"

extern QStringList tagslist;
extern QCompleter* reason_name_completer;


constexpr const char* subreddit_a1 = 
    "SELECT S.name, S.id, c.id, c.created_at, c.content, u.name, '' as reason " // Dummy column '' to substitute for 'reason' column
    "FROM comment c, user u "
    "JOIN ("
        "SELECT R.name, s.id "
        "FROM submission s "
        "JOIN ( "
            "SELECT r.id, r.name "
            "FROM subreddit r ";
constexpr const char* subreddit_b1 = 
            "JOIN ( "
                "SELECT s2t.subreddit_id "
                "FROM subreddit2tag s2t "
                "JOIN ("
                    "SELECT t.id "
                    "FROM tag t "
                    "WHERE t.name=\"";

constexpr const char* subreddit_b2 = 
                "\") T on T.id = s2t.tag_id "
            ") S2T on S2T.subreddit_id = r.id ";
constexpr const char* subreddit_a2 = 
        ") R on R.id = s.subreddit_id "
    ") S on S.id = c.submission_id "
    "WHERE u.id=c.author_id";

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


ViewMatchedComments::ViewMatchedComments(QWidget* parent) : QWidget(parent), res1(0) {
    QVBoxLayout* l = new QVBoxLayout(this);
    
    l->addWidget(new QLabel("Tag Name:", this));
    this->tagname_input = new QLineEdit(this);
    QCompleter* tagcompleter = new QCompleter(tagslist);
    this->tagname_input->setCompleter(tagcompleter);
    l->addWidget(this->tagname_input);
    
    l->addWidget(new QLabel("Reason Matched:", this));
    this->reasonname_input = new QLineEdit(this);
    this->reasonname_input->setCompleter(reason_name_completer);
    l->addWidget(this->reasonname_input);
    
    QPushButton* next_btn = new QPushButton("Query", this);
    l->addWidget(next_btn);
    connect(next_btn, &QPushButton::clicked, this, &ViewMatchedComments::init);
    
    QPushButton* init_btn = new QPushButton("Next", this);
    l->addWidget(init_btn);
    connect(init_btn, &QPushButton::clicked, this, &ViewMatchedComments::next);
    
    this->subname   = new QLabel(this);
    this->username  = new QLabel(this);
    this->reasonname = new QLabel(this);
    this->datetime  = new QLabel(this);
    
    l->addWidget(this->subname);
    l->addWidget(this->username);
    l->addWidget(this->reasonname);
    l->addWidget(this->datetime);
    
    this->textarea = new QPlainTextEdit(this);
    this->textarea->setReadOnly(true);
    
    l->addWidget(this->textarea);
    
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
    const QString tag    = this->tagname_input->text();
    const QString reason = this->reasonname_input->text();
    
    constexpr static const compsky::asciify::flag::ChangeBuffer chbuf;
    compsky::asciify::asciify(chbuf, compsky::asciify::BUF, 0);
    
    if (!tag.isEmpty())
        if (!reason.isEmpty())
            compsky::asciify::asciify(subreddit_a1, subreddit_b1, tag, subreddit_b2, subreddit_a2);
        else
            compsky::asciify::asciify(subreddit_a1, tag, subreddit_a2);
    if (!reason.isEmpty())
        compsky::asciify::asciify(reason_a1, reason_b1, reason, reason_b2, reason_a2);
    else
        // TODO: Clean up current contents
        return;
    
    compsky::mysql::query_buffer(&this->res1, compsky::asciify::BUF, compsky::asciify::BUF_INDX);
    
    this->next();
}

void ViewMatchedComments::next(){
    if (this->res1 == nullptr)
        return;
    char* subname;
    constexpr static const compsky::asciify::flag::StrLen f;
    char* body;
    uint64_t post_id;
    uint64_t cmnt_id;
    size_t body_sz;
    uint64_t t;
    char* username;
    char* reason;
    char dt_buf[200];
    struct tm* dt;
    if (compsky::mysql::assign_next_row(this->res1, &this->row1, &subname, &post_id, &cmnt_id, &t, f, &body_sz, &body, &username, &reason)){
        post_id_str[id2str(post_id, post_id_str)] = 0;
        cmnt_id_str[id2str(cmnt_id, cmnt_id_str)] = 0;
        
        const time_t tt = t;
        dt = localtime(&tt);
        strftime(dt_buf, sizeof(dt_buf), "%Y %a %b %d %H:%M:%S", dt);
        
        this->subname->setText(subname);
        this->username->setText(username);
        this->reasonname->setText(reason);
        this->datetime->setText(dt_buf);
        
        this->textarea->setPlainText(body);
    }
}
