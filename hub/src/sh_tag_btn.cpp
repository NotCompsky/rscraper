/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "sh_tag_btn.hpp"

#include <QCompleter>
#include <QMessageBox>
#include <QStringList>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"
#include "name_dialog.hpp"


extern QStringList category_names;


ShTagBtn::ShTagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("Share", parent) {}

void ShTagBtn::exec(){
    NameDialog* namedialog = new NameDialog("Share with category", "");
    namedialog->name_edit->setCompleter(new QCompleter(category_names));
    
    if (namedialog->exec() != QDialog::Accepted)
            return;
        const QString qstr = namedialog->name_edit->text();
        if (qstr.isEmpty())
            return;
    
    constexpr static const compsky::asciify::flag::Escape f_esc;
    compsky::mysql::exec("INSERT IGNORE INTO tag2category SELECT ", this->tag_id, ", id FROM category WHERE name=\"", f_esc, '"', qstr, "\"");
    QMessageBox::information(this,  "Success",  "The tag has been shared, but will not appear in other categories until rscraper-hub is restarted");
}

void ShTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->exec();
    }
}
