#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QString>
#include <QVBoxLayout>
#include <inttypes.h> // for uintN_t


struct tag2cl{
    uint64_t tagid;
    char* tagname;
    uint32_t cl;
};

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

class ClTagsTab : public QWidget{
    Q_OBJECT
  public:
    explicit ClTagsTab(const uint64_t id, QWidget* parent = 0);
    const uint64_t cat_id;
  public Q_SLOTS:
    void add_tag();
    uint64_t create_tag(QString& qs,  const char* s);
  private:
    QVBoxLayout* l;
};

class ClTagsDialog : public QDialog{
    Q_OBJECT
  public:
    ~ClTagsDialog();
    explicit ClTagsDialog(QWidget* parent = 0);
};

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
