/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include <QColor>
#include <QMouseEvent>
#include <QPushButton>


class SelectColourButton : public QPushButton{
    Q_OBJECT
  private:
    void display_subs_w_tag();
  private Q_SLOTS:
    void mousePressEvent(QMouseEvent* e);
  public:
    uint64_t tag_id; // Should be const
    QColor colour;
    explicit SelectColourButton(const uint64_t id,  const unsigned char r,  const unsigned char g,  const unsigned char b,  const unsigned char a,  const char* name,  QWidget* parent);
  public Q_SLOTS:
    void set_colour();
};
