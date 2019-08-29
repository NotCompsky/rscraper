/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "btn_with_id.hpp"
#include "mysql_declarations.hpp"
#include "categorytab.hpp"
#include "name_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QCompleter>
#include <QMessageBox>
#include <QStringList>


extern QStringList category_names;


BtnWithID::BtnWithID(const QString& title,  const uint64_t id_,  QWidget* parent)
: QPushButton(title, parent)
, id(id_)
{}

void BtnWithID::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			emit(left_clicked(this->id));
		default: return;
	}
}
