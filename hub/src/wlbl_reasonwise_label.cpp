/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "wlbl_reasonwise_label.hpp"

#include <QMessageBox>

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}


WlBlReasonwiseLabel::WlBlReasonwiseLabel(const char* name,  const char* typ,  const char* typ_id_varname,  const char* tblname)
:
    WlBlLabel(name, typ, typ_id_varname, tblname)
{}


void WlBlReasonwiseLabel::display_subs_w_tag(){
    compsky::mysql::query(&RES1,  "SELECT m.name,b.name FROM reason_matched m,", this->tblname, " a,", this->typ, " b WHERE m.id=a.reason AND a.", this->typ_id_varname, "=b.id");
    
    char* reason_name;
    char* subreddit_name;
    QString s = this->text();
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &reason_name, &subreddit_name)){
        s += '\n';
        s += reason_name;
        s += '\t';
        s += subreddit_name;
    }
    
    QMessageBox* msgbox = new QMessageBox(this);
    msgbox->setText(this->tblname);
    msgbox->setWindowModality(Qt::NonModal);
    msgbox->setInformativeText(s);
    msgbox->exec();
}
