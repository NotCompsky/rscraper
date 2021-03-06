/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "add_sub2tag_btn.hpp"
#include "mysql_declarations.hpp"
#include "notfound.hpp"
#include "sql_name_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QCompleter>


extern QStringList subreddit_names;
extern QCompleter* subreddit_name_completer;


AddSub2TagBtn::AddSub2TagBtn(const uint64_t id,  const bool _delete_from,  QWidget* parent)
: QPushButton(QString(_delete_from?"-":"+") + "Subreddits",  parent)
, delete_from(_delete_from)
, tag_id(id)	
{}

void AddSub2TagBtn::add_subreddit(){
	while(true){
		SQLNameDialog* namedialog = new SQLNameDialog("Subreddit Name");
		
		if (!this->delete_from)
			namedialog->name_edit->setCompleter(subreddit_name_completer);
		else {
			QStringList tag_subreddits_names;
			compsky::mysql::query(_mysql::obj, _mysql::res1, BUF, "SELECT name FROM subreddit s, subreddit2tag s2t WHERE s2t.subreddit_id=s.id AND s2t.tag_id=", this->tag_id);
			const char* name;
			while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &name))
				tag_subreddits_names << name;
			QCompleter* tag_subreddits_names_completer = new QCompleter(tag_subreddits_names, namedialog);
			namedialog->name_edit->setCompleter(tag_subreddits_names_completer);
		}
		
		const int rc = namedialog->exec();
		char opening_char;
		char closing_char;
		char escape_char;
		const char* patternstr;
		namedialog->interpret(patternstr, opening_char, closing_char, escape_char);
		const bool is_pattern = !(patternstr[0] == '=');
		const QString qstr = namedialog->name_edit->text();
		
		delete namedialog;
		
		if (rc != QDialog::Accepted)
			return;
		
		if (qstr.isEmpty())
			return;
		
		
		
		if (!is_pattern  &&  !subreddit_names.contains(qstr))
			return notfound::subreddit(this, qstr);
		
		compsky::mysql::exec(_mysql::obj, BUF,
			(this->delete_from) ? "DELETE s2t FROM subreddit2tag s2t LEFT JOIN subreddit s ON s2t.subreddit_id=s.id WHERE tag_id=" : "INSERT IGNORE INTO subreddit2tag SELECT id,",
			this->tag_id,
			(this->delete_from) ? " AND s.name" : " FROM subreddit WHERE name",
			patternstr,
			opening_char,  qstr,  closing_char
		);
	}
}

void AddSub2TagBtn::mousePressEvent(QMouseEvent* e){
	switch(e->button()){
		case Qt::LeftButton:
			return this->add_subreddit();
		default: return;
	}
}
