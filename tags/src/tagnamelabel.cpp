/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "tagnamelabel.hpp"

#include <QMessageBox>

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include "tagdialog.hpp"


extern QStringList tagslist;
extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}


TagNameLabel::TagNameLabel(const uint64_t tag_id,  char* name,  QWidget* parent) : tag_id(tag_id), QLabel(name, parent) {
}

TagNameLabel::TagNameLabel(const uint64_t tag_id,  QString& qname,  QWidget* parent) : tag_id(tag_id), QLabel(qname, parent) {
}


void TagNameLabel::rename_tag(){
    bool ok;
    TagDialog* tagdialog = new TagDialog("Rename Tag", "");
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    QString tagstr = tagdialog->name_edit->text();
    if (tagstr.isEmpty())
        return;
    if (tagstr == this->text())
        return;
    
    const QByteArray ba = tagstr.toLocal8Bit();
    const char* tag_str = ba.data();
    
    tagslist[tagslist.indexOf(this->text())] = tagstr;
    
    compsky::mysql::exec("UPDATE tag SET name=\"", _f::esc, '"', tag_str, "\" WHERE id=",  this->tag_id);
    
    this->setText(tagstr);
}


void TagNameLabel::display_subs_w_tag(){
    compsky::mysql::query(&RES1,  "SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id=",  this->tag_id,  ") A ON A.subreddit_id = r.id");
    
    char* name;
    QString DISPLAY_TAGS_RES = "";
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
        DISPLAY_TAGS_RES += name;
        DISPLAY_TAGS_RES += '\n';
    }
    
    QMessageBox::information(this, tr("Tagged Subreddits"), DISPLAY_TAGS_RES, QMessageBox::Cancel);
}


void TagNameLabel::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            this->rename_tag();
            return;
        case Qt::RightButton:
            this->display_subs_w_tag();
            return;
    }
}
