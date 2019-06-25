/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QDialog>

#include <inttypes.h> // for uintN_t


struct tag2cl{
    uint64_t tagid;
    char* tagname;
    uint32_t cl;
};




class MainWindow : public QDialog{
    Q_OBJECT
  public:
    ~MainWindow();
    explicit MainWindow(QWidget* parent = 0);
  private Q_SLOTS:
    void rename_category(int indx);
  private:
    QTabWidget* tab_widget;
};
#endif
