/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "clbtn.hpp"

#include <QColorDialog>
#include <QMessageBox>
#include <QPalette>

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


SelectColourButton::SelectColourButton(const uint64_t id,  const unsigned char r,  const unsigned char g,  const unsigned char b,  const unsigned char a,  const char* name,  QWidget* parent){
    this->tag_id = id;
    this->setText(name);
    this->setAutoFillBackground(true);
    this->setFlat(true);
    this->colour = QColor(r, g, b, a);
    
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, this->colour);
    this->setPalette(pal);
    this->update();
}

void SelectColourButton::set_colour(){
    this->colour = QColorDialog::getColor(this->colour, parentWidget());
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, this->colour);
    this->setPalette(pal);
    this->update();
    
    int ir, ig, ib, ia;
    this->colour.getRgb(&ir, &ig, &ib, &ia);
    
    const double r = ir/255.0;
    const double g = ig/255.0;
    const double b = ib/255.0;
    const double a = ia/255.0;
    
    constexpr static const compsky::asciify::flag::guarantee::BetweenZeroAndOneInclusive f;
    
    compsky::mysql::exec("UPDATE tag SET r=", f, r, 3, ",g=", f, g, 3, ",b=", f, b, 3, ",a=", f, a, 3, " WHERE id=", this->tag_id);
}

void SelectColourButton::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            this->set_colour();
            return;
        case Qt::RightButton:
            this->display_subs_w_tag();
            return;
    }
}

void SelectColourButton::display_subs_w_tag(){
    compsky::mysql::query(&RES1,  "SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id=",  this->tag_id,  ") A ON A.subreddit_id = r.id");
    
    char* name;
    QString DISPLAY_TAGS_RES = "";
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
        DISPLAY_TAGS_RES += name;
        DISPLAY_TAGS_RES += '\n';
    }
    
    QMessageBox::information(this, tr("Tagged Subreddits"), DISPLAY_TAGS_RES, QMessageBox::Cancel);
}
