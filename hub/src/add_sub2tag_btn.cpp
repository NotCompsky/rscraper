/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "add_sub2tag_btn.hpp"

#include <QCompleter>

#include "name_dialog.hpp"
#include "notfound.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QStringList subreddit_names;
extern QCompleter* subreddit_name_completer;


AddSub2TagBtn::AddSub2TagBtn(const uint64_t id,  bool delete_from,  QWidget* parent)
:
    tag_id(id),
    QPushButton(QString(delete_from?"-":"+") + "Subreddits",  parent),
    delete_from(delete_from)
{}

void AddSub2TagBtn::add_subreddit(){
    bool ok;
    while(true){
        NameDialog* namedialog = new NameDialog("Subreddit Name", "", "Use SQL LIKE pattern matching");
        
        if (!this->delete_from)
            namedialog->name_edit->setCompleter(subreddit_name_completer);
        else {
            QStringList tag_subreddits_names;
            compsky::mysql::query(&RES1, "SELECT name FROM subreddit s, subreddit2tag s2t WHERE s2t.subreddit_id=s.id AND s2t.tag_id=", this->tag_id);
            char* name;
            while(compsky::mysql::assign_next_row(RES1, &ROW1, &name))
                tag_subreddits_names << name;
            QCompleter* tag_subreddits_names_completer = new QCompleter(tag_subreddits_names, namedialog);
            namedialog->name_edit->setCompleter(tag_subreddits_names_completer);
        }
        
        if (namedialog->exec() != QDialog::Accepted)
            return;
        const QString qstr = namedialog->name_edit->text();
        if (qstr.isEmpty())
            return;
        
        const bool is_pattern = namedialog->checkbox->isChecked();
        
        if (!is_pattern  &&  !subreddit_names.contains(qstr))
            return notfound::subreddit(this, qstr);
        
        const QByteArray ba = qstr.toLocal8Bit();
        const char* subreddit_name = ba.data();
        
        // TODO: Add QCompleter for subreddit name
        
        compsky::mysql::exec(
            (this->delete_from) ? "DELETE s2t FROM subreddit2tag s2t LEFT JOIN subreddit s ON s2t.subreddit_id=s.id WHERE tag_id=" : "INSERT IGNORE INTO subreddit2tag SELECT id,",
            this->tag_id,
            (this->delete_from) ? " AND s.name" : " FROM subreddit WHERE name",
            (is_pattern)?" LIKE ":"=",
            '"',  subreddit_name,  '"'
        );
    }
}

void AddSub2TagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->add_subreddit();
    }
}
