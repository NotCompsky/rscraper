/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __CATEGORYTAB_H__
#define __CATEGORYTAB_H__

#include <QGridLayout>
#include <QTabWidget>
#include <QWidget>


class ClTagsTab : public QWidget{
    Q_OBJECT
  public:
    explicit ClTagsTab(const uint64_t id,  QTabWidget* tab_widget,  QWidget* parent = 0);
    const uint64_t cat_id;
  public Q_SLOTS:
    void add_tag();
    void rm_self();
    uint64_t create_tag(const QString& qs);
  private:
    void display_tag_stats(const int tag_id);
    void add_tag_row(const uint64_t id,  QString name,  const double r,  const double g,  const double b,  const double a);
    QGridLayout* l;
    QTabWidget* tab_widget;
    int row;
};

#endif
