#ifndef __CAT_DOUGHNUT_H__
#define __CAT_DOUGHNUT_H__

#include <QDialog>


class CatDoughnut : public QDialog {
    Q_OBJECT
  public:
    explicit CatDoughnut(QWidget* parent);
    void show_chart();
  private:
    void init();
    bool is_initialised;
};


#endif
