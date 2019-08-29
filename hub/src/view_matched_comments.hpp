/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_HUB_VIEW_MATCHED_COMMENTS_HPP
#define RSCRAPER_HUB_VIEW_MATCHED_COMMENTS_HPP

#include <compsky/mysql/mysql.hpp>

#include <QCheckBox>
#include <QRadioButton>
#include <QLabel>
#include <QPlainTextEdit>
#include <QStringList>
#include <QLineEdit>
#include <QWidget>


class ViewMatchedComments : public QWidget {
  public:
	explicit ViewMatchedComments(QWidget* parent);
	~ViewMatchedComments();
  private:
	void next();
	void ch_reason();
	void del_cmnt();
	void generate_query();
	void execute_query();
	void toggle_order_btns();
	void view_matches();
	const char* get_sort_column();
	void reset_index();
	char buf[4096];
	char* itr;
	int query_indx;
	int cached_cmnt_indx;
	QStringList cmnt_contents_from_remote;
	QLineEdit* tagname_input;
	QLineEdit* reasonname_input;
	QLineEdit* limit_input;
	QPlainTextEdit* textarea;
	QLineEdit* ch_reason_input;
	QLabel* subname;
	QLabel* username;
	QLabel* reasonname;
	QLabel* datetime;
	QLineEdit* permalink;
	QRadioButton* sorting_column_btns[6];
	char post_id_str[10];
	char cmnt_id_str[10];
	uint64_t cmnt_id;
	const char* cmnt_body;
	size_t cmnt_body_sz;
	MYSQL_RES* res1;
	MYSQL_ROW row1;
	QPlainTextEdit* query_text;
	QCheckBox* get_empty_comments;
	QCheckBox* is_content_from_remote;
	bool is_ascending;
};


#endif
