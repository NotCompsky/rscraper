/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "unlink_tag_btn.hpp"

#include <QMessageBox>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"


UnlinkTagBtn::UnlinkTagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("Unlink", parent) {}

void UnlinkTagBtn::exec(){
    compsky::mysql::exec("DELETE FROM tag2category WHERE tag_id=", this->tag_id, " AND category_id=", static_cast<ClTagsTab*>(this->parent())->cat_id);
    QMessageBox::information(this,  "Success",  "The tag has been unlinked, but will still appear in this category until rscraper-hub is restarted");
}

void UnlinkTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->exec();
    }
}
