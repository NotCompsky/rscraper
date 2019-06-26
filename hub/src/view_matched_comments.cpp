/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "view_matched_comments.hpp"

#include <ctime> // for strftime, localtime, time_t

#include <QCompleter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QRadioButton>
#include <QVBoxLayout>

#include "id2str.hpp"

extern QStringList tagslist;
extern QCompleter* reason_name_completer;


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


ViewMatchedComments::ViewMatchedComments(QWidget* parent) : QWidget(parent), res1(0), is_ascending(false) {
    QVBoxLayout* l = new QVBoxLayout;
    
    l->addWidget(new QLabel("Tag Name:", this));
    this->tagname_input = new QLineEdit(this);
    QCompleter* tagcompleter = new QCompleter(tagslist);
    this->tagname_input->setCompleter(tagcompleter);
    l->addWidget(this->tagname_input);
    
    l->addWidget(new QLabel("Reason Matched:", this));
    this->reasonname_input = new QLineEdit(this);
    this->reasonname_input->setCompleter(reason_name_completer);
    l->addWidget(this->reasonname_input);
    
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
        
        this->permalink->setText(QString("https://www.reddit.com/r/" + QString(subname) + QString("/comments/") + QString(post_id_str) + QString("/_/") + QString(cmnt_id_str)));
        
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
#include <QMessageBox>
void ViewMatchedComments::toggle_order_btns(){
    this->is_ascending = !this->is_ascending;
    QMessageBox::information(this, "is_ascending",  (this->is_ascending) ? "true" : "false");
}
