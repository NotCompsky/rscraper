/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_HUB_WLBL_LABEL_HPP
#define RSCRAPER_HUB_WLBL_LABEL_HPP

#include <QLabel>
#include <QMouseEvent>

class WlBlLabel : public QLabel {
    Q_OBJECT
  private:
    virtual void display_subs_w_tag();
  private Q_SLOTS:
    void mousePressEvent(QMouseEvent* e);
  public:
    const char* tblname;
    const char* typ;
    const char* typ_id_varname; // Most likely "id".
    explicit WlBlLabel(const char* title,  const char* typ,  const char* typ_id_varname,  const char* tblname);
};


#endif
