/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "categorytab.hpp"

#include <map>

#include <QDialogButtonBox>

#include <compsky/mysql/query.hpp>

#include "add_sub2tag_btn.hpp"
#include "rm_sub2tag_btn.hpp"
#include "rm_tag_btn.hpp"
#include "clbtn.hpp"
#include "tagdialog.hpp"
#include "tagnamelabel.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;

extern std::map<QString, uint64_t> tag_name2id;
extern QStringList tagslist;


ClTagsTab::ClTagsTab(const uint64_t cat_id, QWidget* parent) : cat_id(cat_id), QWidget(parent), row(0){
    this->l = new QGridLayout;
    
    QPushButton* add_tag_btn = new QPushButton("+", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_tag()));
    this->l->addWidget(add_tag_btn, 0, 0);
    
    compsky::mysql::query(&RES2,  "SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id=",  cat_id,  ") ORDER BY name");
    
    {
    uint64_t id;
    char* name;
    unsigned char r, g, b, a;
    
    while (compsky::mysql::assign_next_row(RES2, &ROW2, &id, &name, &r, &g, &b, &a)){
        ++this->row;
        this->l->addWidget(new TagNameLabel(id, name, this),  this->row,  0);
        this->l->addWidget(new SelectColourButton(id, r, g, b, a, this),  this->row,  1);
        this->l->addWidget(new AddSub2TagBtn(id, this),  this->row,  2);
        this->l->addWidget(new RmSub2TagBtn(id, this),   this->row,  3);
        this->l->addWidget(new RmTagBtn(id, this),       this->row,  4);
    }
    }
    
    setLayout(this->l);
}

uint64_t ClTagsTab::create_tag(QString& qs,  const char* s){
    constexpr static const compsky::asciify::flag::Escape f;
    compsky::mysql::exec("INSERT INTO tag (name, r,g,b,a) VALUES (\"",  f,  '"',  s,  "\",0,0,0,0)");
    compsky::mysql::query_buffer(&RES1,  "SELECT LAST_INSERT_ID() as ''");
    uint64_t id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &id));
    tag_name2id[qs] = id;
    return id;
}

void ClTagsTab::add_tag(){
    bool ok;
    TagDialog* tagdialog = new TagDialog("Tag", "");
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    QString tagstr = tagdialog->name_edit->text();
    if (tagstr.isEmpty())
        return;
    
    QByteArray ba = tagstr.toLocal8Bit();
    char* tag_str = ba.data();
    
    const uint64_t tag_id  =  (tagslist.contains(tagstr))  ?  tag_name2id[tagstr]  :  this->create_tag(tagstr, tag_str);
    
    compsky::mysql::exec("INSERT INTO tag2category (category_id, tag_id) VALUES (",  this->cat_id,  ',',  tag_id,  ')');
    ++this->row;
    this->l->addWidget(new TagNameLabel(tag_id, tagstr, this),  this->row,  0);
    this->l->addWidget(new SelectColourButton(tag_id, 0, 0, 0, 0, this),  this->row,  1);
    this->l->addWidget(new AddSub2TagBtn(tag_id, this),  this->row,  2);
    this->l->addWidget(new RmSub2TagBtn(tag_id, this),   this->row,  3);
    this->l->addWidget(new RmTagBtn(tag_id, this),       this->row,  4);
}
