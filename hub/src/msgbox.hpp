#ifndef __MSGBOX_H__
#define __MSGBOX_H__

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
