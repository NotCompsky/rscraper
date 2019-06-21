/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "add_sub2tag_btn.hpp"

#include "subreddit_name_dialog.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


AddSub2TagBtn::AddSub2TagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("+Subreddits", parent) {}

void AddSub2TagBtn::add_subreddit(){
    bool ok;
    while(true){
        SubredditNameDialog* namedialog = new SubredditNameDialog("Subreddit Name", "");
        if (namedialog->exec() != QDialog::Accepted)
            return;
        const QString qstr = namedialog->name_edit->text();
        if (qstr.isEmpty())
            return;
        
        const QByteArray ba = qstr.toLocal8Bit();
        const char* subreddit_name = ba.data();
        
        // TODO: Add QCompleter for subreddit name
        
        compsky::mysql::exec("INSERT IGNORE INTO subreddit2tag SELECT id,",  this->tag_id,  " FROM subreddit WHERE name=\"",  subreddit_name,  "\"");
    }
}

void AddSub2TagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->add_subreddit();
    }
}
