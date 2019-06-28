/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "mv_tag_btn.hpp"

#include <QCompleter>
#include <QMessageBox>
#include <QStringList>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"
#include "name_dialog.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QStringList category_names;


MvTagBtn::MvTagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("Move", parent) {}

void MvTagBtn::exec(){
    NameDialog* namedialog = new NameDialog("Move to category", "");
    namedialog->name_edit->setCompleter(new QCompleter(category_names));
    
    if (namedialog->exec() != QDialog::Accepted)
            return;
        const QString qstr = namedialog->name_edit->text();
        if (qstr.isEmpty())
            return;
    
    constexpr static const compsky::asciify::flag::Escape f_esc;
    compsky::mysql::exec("UPDATE IGNORE tag2category t2c, category c SET t2c.category_id=c.id WHERE t2c.tag_id=", this->tag_id, " AND t2c.category_id=", static_cast<ClTagsTab*>(this->parent())->cat_id, " AND c.name=\"", f_esc, '"', qstr, "\"");
    compsky::mysql::exec("DELETE t2c FROM tag2category t2c, category c WHERE t2c.tag_id=", this->tag_id, " AND t2c.category_id=", static_cast<ClTagsTab*>(this->parent())->cat_id);
    
    // NOTE: Aim to support tags being in *multiple* categories - i.e. t2c.tag_id not being the primary key - despite this GUI not currently having a way to create that situation.
    
    QMessageBox::information(this,  "Success",  "The tag has been moved, but will still appear in this category until rscraper-hub is restarted");
}

void MvTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->exec();
    }
}
