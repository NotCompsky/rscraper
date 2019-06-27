/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "btn_with_id.hpp"

#include <QCompleter>
#include <QMessageBox>
#include <QStringList>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"
#include "name_dialog.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QStringList category_names;


BtnWithID::BtnWithID(const QString& title,  const uint64_t id,  QWidget* parent) : id(id), QPushButton(title, parent) {}

void BtnWithID::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            emit(left_clicked(this->id));
    }
}
