/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "rm_tag_btn.hpp"

#include <QGridLayout>

#include "add_sub2tag_btn.hpp"
#include "rm_sub2tag_btn.hpp"
#include "clbtn.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


RmTagBtn::RmTagBtn(const uint64_t id,  QWidget* parent) : tag_id(id), QPushButton("Delete", parent) {}

void RmTagBtn::rm_tag(){
    compsky::mysql::exec("DELETE FROM subreddit2tag WHERE tag_id=", this->tag_id);
    compsky::mysql::exec("DELETE FROM tag WHERE id=", this->tag_id);
    const QWidget* par = reinterpret_cast<QWidget*>(this->parent());
    QGridLayout* l = reinterpret_cast<QGridLayout*>(par->layout());
    int row, col, rowspan, colspan;
    l->getItemPosition(l->indexOf(this), &row, &col, &rowspan, &colspan);
    QLayoutItem* a = l->itemAtPosition(row, 0);
    QLayoutItem* b = l->itemAtPosition(row, 1);
    QLayoutItem* c = l->itemAtPosition(row, 2);
    delete a->widget();
    delete b->widget();
    delete c->widget();
    l->removeItem(a);
    l->removeItem(b);
    l->removeItem(c);
    l->removeItem(reinterpret_cast<QLayoutItem*>(this));
    delete this; // Suicide is fine, it was guaranteed allocated by new RmTagBtn
}

void RmTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->rm_tag();
    }
}
