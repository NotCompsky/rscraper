/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "rm_tag_btn.hpp"

#include <QGridLayout>
#include <QMessageBox>

#include "clbtn.hpp"

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


RmTagBtn::RmTagBtn(const uint64_t id,  QWidget* parent) : QPushButton("Delete", parent), tag_id(id) {
    QPalette palette = this->palette();
    palette.setColor(QPalette::Button, QColor(Qt::red));
    this->setAutoFillBackground(true);
    this->setPalette(palette);
    this->setFlat(true);
    this->update();
}

void RmTagBtn::rm_tag(){
    compsky::mysql::exec("DELETE FROM subreddit2tag WHERE tag_id=", this->tag_id);
    compsky::mysql::exec("DELETE FROM tag WHERE id=", this->tag_id);
    compsky::mysql::exec("DELETE FROM tag2category WHERE tag_id=", this->tag_id);
    
    QMessageBox::information(this,  "Success",  "The tag has been deleted, but will still appear in all its previous categories until rscraper-hub is restarted");
    
    const QWidget* par = reinterpret_cast<QWidget*>(this->parent());
    QGridLayout* l = reinterpret_cast<QGridLayout*>(par->layout());
    int row, col, rowspan, colspan;
    l->getItemPosition(l->indexOf(this), &row, &col, &rowspan, &colspan); // Last position
    
    for (auto i = 0;  i < 8;  ++i){
        QLayoutItem* a = l->itemAtPosition(row, i);
        delete a->widget();
        l->removeItem(a);
    }
}

void RmTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->rm_tag();
        default: return;
    }
}
