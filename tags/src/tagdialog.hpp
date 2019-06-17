/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef __TAGDIALOG__
#define __TAGDIALOG__

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>


class TagDialog : public QDialog {
  public:
    explicit TagDialog(QString title,  QString str,  QWidget* parent = 0);
    ~TagDialog();
    QLineEdit* name_edit;
  private:
    QDialogButtonBox* btn_box;
    QVBoxLayout* l;
};

#endif
