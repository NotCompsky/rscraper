#ifndef __TAGDIALOG__
#define __TAGDIALOG__

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>


class TagDialog : public QDialog {
  public:
    explicit TagDialog(QString title,  QString str,  QWidget* parent = 0);
    ~TagDialog();
    QLineEdit* name_edit;
  private:
    QDialogButtonBox* btn_box;
    QVBoxLayout* l;
};

#endif
