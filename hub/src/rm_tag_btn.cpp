/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "rm_tag_btn.hpp"
#include "mysql_declarations.hpp"
#include "categorytab.hpp"
#include "clbtn.hpp"
#include "unlink_tag_btn.hpp"

#include <compsky/mysql/query.hpp>

#include <QGridLayout>
#include <QMessageBox>


RmTagBtn::RmTagBtn(const uint64_t id,  QWidget* parent) : QPushButton("Delete", parent), tag_id(id) {
	QPalette palette = this->palette();
	palette.setColor(QPalette::Button, QColor(Qt::red));
	this->setAutoFillBackground(true);
	this->setPalette(palette);
	this->setFlat(true);
	this->update();
}

void RmTagBtn::rm_tag(){
	compsky::mysql::exec(_mysql::obj, BUF, "DELETE FROM subreddit2tag WHERE tag_id=", this->tag_id);
	compsky::mysql::exec(_mysql::obj, BUF, "DELETE FROM tag WHERE id=", this->tag_id);
	compsky::mysql::exec(_mysql::obj, BUF, "DELETE FROM tag2category WHERE tag_id=", this->tag_id);
	
	ClTagsTab* cat_tab = reinterpret_cast<ClTagsTab*>(this->parent());
	const int indx = cat_tab->l->indexOf(this);
	
	int row, col, rowspan, colspan;
	cat_tab->l->getItemPosition(indx, &row, &col, &rowspan, &colspan);
	
	QMessageBox::information(this,  "Success",  "The tag has been deleted, but will still appear in other previous categories - if it was assigned to them - until rscraper-hub is restarted");
	
	reinterpret_cast<UnlinkTagBtn*>(cat_tab->l->itemAtPosition(row, 6)->widget())->exec(); // unlink, which removes the row from the layout (includes deleting this object) // TODO: Rename exec to unlink_tag, for safety
}

void RmTagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->rm_tag();
		default: return;
	}
}
