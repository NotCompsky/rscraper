#ifndef RSCRAPER_HUB_CAT_DOUGHNUT_HPP
#define RSCRAPER_HUB_CAT_DOUGHNUT_HPP

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
