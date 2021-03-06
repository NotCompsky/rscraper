/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "tagnamelabel.hpp"
#include "mysql_declarations.hpp"
#include "msgbox.hpp"
#include "name_dialog.hpp"

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <QCompleter>


extern QStringList tagslist;


TagNameLabel::TagNameLabel(const uint64_t tag_id_,  char* name,  QWidget* parent)
: QLabel(name, parent)
, tag_id(tag_id_)
{}

TagNameLabel::TagNameLabel(const uint64_t tag_id_,  QString& qname,  QWidget* parent)
: QLabel(qname, parent)
, tag_id(tag_id_)
{}


void TagNameLabel::rename_tag(){
	NameDialog* tagdialog = new NameDialog("Rename Tag", this->text());
	QCompleter* tagcompleter = new QCompleter(tagslist);
	tagdialog->name_edit->setCompleter(tagcompleter);
	const int rc = tagdialog->exec();
	QString tagstr = tagdialog->name_edit->text();
	delete tagdialog;
	if (rc != QDialog::Accepted  ||  tagstr.isEmpty()  ||  tagstr == this->text())
		return;
	
	const QByteArray ba = tagstr.toLocal8Bit();
	const char* tag_str = ba.data();
	
	tagslist[tagslist.indexOf(this->text())] = tagstr;
	
	compsky::mysql::exec(_mysql::obj, BUF, "UPDATE tag SET name=\"", _f::esc, '"', tag_str, "\" WHERE id=",  this->tag_id);
	
	this->setText(tagstr);
}


void TagNameLabel::display_subs_w_tag(){
	compsky::mysql::query(_mysql::obj, _mysql::res1, BUF,  "SELECT r.name FROM subreddit r, subreddit2tag s WHERE s.subreddit_id=r.id AND  s.tag_id=",  this->tag_id,  " ORDER BY r.name");
	
	const char* name;
	QString DISPLAY_TAGS_RES = "";
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &name)){
		DISPLAY_TAGS_RES += name;
		DISPLAY_TAGS_RES += '\n';
	}
	
	MsgBox* msgbox = new MsgBox(this,  "Tagged Subreddits",  DISPLAY_TAGS_RES);
	msgbox->exec();
}


void TagNameLabel::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			this->rename_tag();
			return;
		case Qt::RightButton:
			this->display_subs_w_tag();
			return;
		default: return;
	}
}
