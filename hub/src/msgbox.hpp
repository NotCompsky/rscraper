#ifndef RSCRAPER_HUB_MSGBOX_HPP
#define RSCRAPER_HUB_MSGBOX_HPP

#include <QMessageBox>
#include <QResizeEvent>
#include <QShowEvent>


class MsgBox : public QMessageBox {
  public:
    explicit MsgBox(QWidget* parent,  const QString& text,  const QString& details,  int w = 0);
  private:
    void showEvent(QShowEvent* e);
    void resizeEvent(QResizeEvent* e);
    int w;
};


#endif
