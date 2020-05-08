/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_HUB_SQL_NAME_DIALOG_HPP
#define RSCRAPER_HUB_SQL_NAME_DIALOG_HPP

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>


class SQLNameDialog : public QDialog {
  public:
	explicit SQLNameDialog(QString title,  QString str = "",  QWidget* parent = 0);
	void interpret(const char*& pattern,  char& opening,  char& closing,  char& escape);
	void interpret(const char*& pattern);
	QLineEdit* name_edit;
  private:
	QRadioButton* pattern_btns[4];
};


#endif
