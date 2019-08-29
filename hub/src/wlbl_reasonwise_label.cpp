/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "wlbl_reasonwise_label.hpp"
#include "msgbox.hpp"
#include "mysql_declarations.hpp"

#include <compsky/mysql/query.hpp>


WlBlReasonwiseLabel::WlBlReasonwiseLabel(const char* name,  const char* typ_,  const char* typ_id_varname_,  const char* tblname_)
:
	WlBlLabel(name, typ_, typ_id_varname_, tblname_)
{}


void WlBlReasonwiseLabel::display_subs_w_tag(){
	compsky::mysql::query(_mysql::obj, _mysql::res1,  BUF,  "SELECT m.name,b.name FROM reason_matched m,", this->tblname, " a,", this->typ, " b WHERE m.id=a.reason AND a.", this->typ_id_varname, "=b.id");
	
	const char* reason_name;
	const char* subreddit_name;
	QString s = this->text();
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &reason_name, &subreddit_name)){
		s += '\n';
		s += reason_name;
		s += '\t';
		s += subreddit_name;
	}
	
	MsgBox* msgbox = new MsgBox(this, this->tblname, s, 720);
	msgbox->exec();
}
