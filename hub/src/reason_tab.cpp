/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "reason_tab.hpp"

#include "clbtn.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


ReasonTab::ReasonTab(QWidget* parent) : QWidget(parent), row(0) {
	this->l = new QGridLayout;
	
	compsky::mysql::query_buffer(&RES1,  "SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM reason_matched ORDER BY name");
	
	{
	uint64_t id;
	char* name;
	uint8_t r, g, b, a;
	
	while (compsky::mysql::assign_next_row(RES1, &ROW1, &id, &name, &r, &g, &b, &a))
		add_tag_row(id, name, QColor(r, g, b, a));
	}
	
	setLayout(this->l);
}

void ReasonTab::add_tag_row(const uint64_t tag_id,  QString tagstr,  const QColor& cl){
	++this->row;
	
	this->l->addWidget(new QLabel(tagstr, this),                                       this->row,  0);
	this->l->addWidget(new SelectColourButton(tag_id,  cl,  this,  "reason_matched"),  this->row,  1);
}
