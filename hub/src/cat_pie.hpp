#ifndef __CAT_PIE_H__
#define __CAT_PIE_H__

#include <QChart>
#include <QDialog>


class CatPie : public QDialog {
    Q_OBJECT
  public:
    explicit CatPie(const uint64_t category_id,  QWidget* parent);
    void show_chart();
    void init();
  private:
    QtCharts::QChart* chart;
    const uint64_t cat_id;
    bool is_initialised;
};


#endif
