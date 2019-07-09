#ifndef RSCRAPER_HUB_TAG_PIE_HPP
#define RSCRAPER_HUB_TAG_PIE_HPP

#include <QChart>
#include <QDialog>


class TagPie : public QDialog {
    Q_OBJECT
  public:
    explicit TagPie(const int tag_id,  QWidget* parent);
};


#endif
