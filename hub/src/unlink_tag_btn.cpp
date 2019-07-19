/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "unlink_tag_btn.hpp"

#include "categorytab.hpp"

#include <compsky/mysql/query.hpp>

#include <QMessageBox>


UnlinkTagBtn::UnlinkTagBtn(const uint64_t id,  QWidget* parent) : QPushButton("Unlink", parent), tag_id(id) {}

void UnlinkTagBtn::exec(){
	compsky::mysql::exec("DELETE FROM tag2category WHERE tag_id=", this->tag_id, " AND category_id=", static_cast<ClTagsTab*>(this->parent())->cat_id);
	
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

void UnlinkTagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->exec();
		default: return;
	}
}
