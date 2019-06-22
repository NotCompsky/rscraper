/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "rm_sub2tag_btn.hpp"

#include "name_dialog.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QCompleter* subreddit_name_completer;


RmSub2TagBtn::RmSub2TagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("-Subreddits", parent) {}

void RmSub2TagBtn::rm_subreddit(){
    bool ok;
    while(true){
        NameDialog* namedialog = new NameDialog("Subreddit Name", "");
        namedialog->name_edit->setCompleter(subreddit_name_completer);
        if (namedialog->exec() != QDialog::Accepted)
            return;
        const QString qstr = namedialog->name_edit->text();
        if (qstr.isEmpty())
            return;
        
        const QByteArray ba = qstr.toLocal8Bit();
        const char* subreddit_name = ba.data();
        
        // TODO: Add QCompleter for subreddit name
        
        compsky::mysql::exec("DELETE s2t FROM subreddit2tag s2t LEFT JOIN subreddit s ON s2t.subreddit_id=s.id WHERE tag_id=", this->tag_id, " AND s.name=\"", subreddit_name, "\"");
    }
}

void RmSub2TagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->rm_subreddit();
    }
}
