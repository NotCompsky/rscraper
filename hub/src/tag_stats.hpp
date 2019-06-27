/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __TAG_STATS_H__
#define __TAG_STATS_H__

#include <QDialog>


class TagStats : public QDialog {
    Q_OBJECT
  public:
    explicit TagStats(uint64_t tag_id,  QWidget* parent);
    void show_chart();
  private:
    void init();
    const uint64_t tag_id;
    bool is_initialised;
};


#endif
