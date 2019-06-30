/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __CATEGORYTAB_H__
#define __CATEGORYTAB_H__

#include <QGridLayout>
#include <QScrollArea>
#include <QTabWidget>
#include <QWidget>


class CatPie;


class ClTagsTab : public QWidget{
    Q_OBJECT
  public:
    explicit ClTagsTab(const uint64_t id,  QTabWidget* tab_widget,  QWidget* parent = 0);
    QGridLayout* l;
    const uint64_t cat_id;
  public Q_SLOTS:
    void add_tag();
    void add_tag_row(const uint64_t tag_id,  QString tagstr,  const QColor& cl);
    void rm_self();
    uint64_t create_tag(const QString& qs);
    QScrollArea* tab_named(const QString& qstr);
  private:
    void display_tag_stats(const int tag_id);
    QTabWidget* tab_widget;
    CatPie* cat_pie;
    int row;
};

#endif
