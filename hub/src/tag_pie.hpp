#ifndef __TAG_PIE_H__
#define __TAG_PIE_H__

#include <QChart>
#include <QDialog>


class TagPie : public QDialog {
    Q_OBJECT
  public:
    explicit TagPie(const int tag_id,  QWidget* parent);
};


#endif
