/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "sql_name_dialog.hpp"

#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QTimer>
#include <QVBoxLayout>


namespace details {
	constexpr static const char* pattern_strs[4] = {"=",  " LIKE ",  " REGEXP ", " IN "};
	constexpr static const char  opening_char[4] = {'"', '"', '"', '('};
	constexpr static const char  closing_char[4] = {'"', '"', '"', ')'};
	constexpr static const char  escape_char[4]  = {'"', '"', '"', 0};
}


SQLNameDialog::SQLNameDialog(QString title,  QString str,  QWidget* parent) : QDialog(parent) {
	// If the functions are implemented in the header file you have to declare the definitions of the functions with inline to prevent having multiple definitions of the functions.
	QDialogButtonBox* btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btn_box, &QDialogButtonBox::accepted, this, &SQLNameDialog::accept);
	connect(btn_box, &QDialogButtonBox::rejected, this, &SQLNameDialog::reject);
	QVBoxLayout* l = new QVBoxLayout;
	l->addWidget(btn_box);
	this->name_edit = new QLineEdit(str, this);
	l->addWidget(this->name_edit);
	
	
	{
	QGroupBox* group_box = new QGroupBox("Pattern Matching");
	this->pattern_btns[0] = new QRadioButton("Literal");
	this->pattern_btns[1] = new QRadioButton("SQL Like");
	this->pattern_btns[2] = new QRadioButton("Regex");
	this->pattern_btns[3] = new QRadioButton("In (SQL)");
	this->pattern_btns[0]->setChecked(true);
	QHBoxLayout* box = new QHBoxLayout;
	box->addWidget(this->pattern_btns[0]);
	box->addWidget(this->pattern_btns[1]);
	box->addWidget(this->pattern_btns[2]);
	box->addWidget(this->pattern_btns[3]);
	box->addStretch(1);
	group_box->setLayout(box);
	l->addWidget(group_box);
	}
	
	
	this->setLayout(l);
	this->setWindowTitle(title);
	QTimer::singleShot(0, this->name_edit, SLOT(setFocus())); // Set focus after SQLNameDialog instance is visible
}

void SQLNameDialog::interpret(const char*& pattern){
	for (auto i = 0;  i < 4;  ++i)
		if (this->pattern_btns[i]->isChecked()){
			pattern = details::pattern_strs[i];
			return;
		}
	// Exactly one radio button is checked at any time. This code path should be impossible.
}

void SQLNameDialog::interpret(const char*& pattern,  char& opening,  char& closing,  char& escape){
	for (auto i = 0;  i < 4;  ++i)
		if (this->pattern_btns[i]->isChecked()){
			pattern = details::pattern_strs[i];
			opening = details::opening_char[i];
			closing = details::closing_char[i];
			escape  = details::escape_char[i];
			return;
		}
	// Exactly one radio button is checked at any time. This code path should be impossible.
}
