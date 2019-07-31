/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "wlbl_label.hpp"

#include <QMessageBox>

#include <compsky/mysql/query.hpp>

#include "msgbox.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}


WlBlLabel::WlBlLabel(const char* name,  const char* typ_,  const char* typ_id_varname_,  const char* tblname_)
: QLabel(name)
, tblname(tblname_)
, typ(typ_)
, typ_id_varname(typ_id_varname_)
{}


void WlBlLabel::display_subs_w_tag(){
	compsky::mysql::query(&RES1,  "SELECT IFNULL(b.name, CONCAT('<ID>', a.id)) FROM ", this->tblname, " a LEFT JOIN ", this->typ, " b ON a.", this->typ_id_varname, "=b.id");
	
	char* name;
	QString s = "";
	while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
		s += '\n';
		s += name;
	}
	
	MsgBox* msgbox = new MsgBox(this, this->text(), s);
	msgbox->exec();
}


void WlBlLabel::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::RightButton:
			this->display_subs_w_tag();
			return;
		default: return;
	}
}
