/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifdef USE_BOOST_REGEX

#include "regex_tests.hpp"
#include "mysql_declarations.hpp"
#include "msgbox.hpp"
#include "sql_name_dialog.hpp"

#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QStringRef>
#include <QHBoxLayout>
#include <QVBoxLayout>


RegexTests::RegexTests(QWidget* parent) : QDialog(parent) {
	QVBoxLayout* l = new QVBoxLayout;
	
	{
		QHBoxLayout* hbox = new QHBoxLayout;

		{
			QPushButton* btn = new QPushButton("Save", this);
			connect(btn, &QPushButton::clicked, this, &RegexTests::save);
			hbox->addWidget(btn);
		}

		l->addLayout(hbox);
	}
	
	this->setLayout(l);
}

void RegexTests::save(){
	
}

#endif
