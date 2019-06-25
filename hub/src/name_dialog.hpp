/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __NAME_DIALOG_H__
#define __NAME_DIALOG_H__

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>


class NameDialog : public QDialog {
  public:
    explicit NameDialog(QString title,  QString str,  QString checkbox_title = "",  QWidget* parent = 0);
    ~NameDialog();
    QLineEdit* name_edit;
    QCheckBox* checkbox;
  private:
    QDialogButtonBox* btn_box;
    QVBoxLayout* l;
};


#endif
