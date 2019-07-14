/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_HUB_REGEX_EDITOR_VARS_MENU_HPP
#define RSCRAPER_HUB_REGEX_EDITOR_VARS_MENU_HPP

#include <QGridLayout>
#include <QScrollArea>
#include <QTabWidget>
#include <QDialog>


void optimise_regex(QString& data,  QString& result);


class RegexEditorVarsMenu : public QDialog {
    Q_OBJECT
  public:
    explicit RegexEditorVarsMenu(QWidget* parent = 0);
    QGridLayout* l;
  public Q_SLOTS:
    void add_var();
    void add_var_row(const QString name,  const QString type_name);
  private Q_SLOTS:
    void var_edit_btn_clicked();
    void var_view_btn_clicked();
    void var_del_btn_clicked();
  private:
    QTabWidget* tab_widget;
    int row;
};

#endif
