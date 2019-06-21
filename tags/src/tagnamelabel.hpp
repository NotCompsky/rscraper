/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __TAGNAMELABEL_H__
#define __TAGNAMELABEL_H__

#include <QLabel>
#include <QMouseEvent>

class TagNameLabel : public QLabel {
    Q_OBJECT
  private:
    void display_subs_w_tag();
    void rename_tag();
    const uint64_t tag_id;
    QString tagname_q;
  private Q_SLOTS:
    void mousePressEvent(QMouseEvent* e);
  public:
    explicit TagNameLabel(const uint64_t tag_id,  char* name,      QWidget* parent);
    explicit TagNameLabel(const uint64_t tag_id,  QString& qname,  QWidget* parent);
};


#endif
