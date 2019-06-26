#ifndef __CAT_DOUGHNUT_H__
#define __CAT_DOUGHNUT_H__

#include <QDialog>


class DonutBreakdownChart;


class CatDoughnut : public QDialog {
    Q_OBJECT
  public:
    explicit CatDoughnut(QWidget* parent);
    void show_chart();
    void init();
  private:
    bool is_initialised;
    DonutBreakdownChart* chart;
};


#endif
