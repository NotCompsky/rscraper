#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>

typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;


struct tag2cl{
    uint64_t tagid;
    char* tagname;
    uint32_t cl;
};

class SelectColourButton : public QPushButton{
    Q_OBJECT
  public:
    uint64_t tag_id; // Should be const
    QColor colour;
    explicit SelectColourButton(const uint64_t id, const double r, const double g, const double b, const double a, const char* name, QWidget* parent);
  public Q_SLOTS:
    void set_colour();
};

class ClTagsTab : public QWidget{
    Q_OBJECT
  public:
    explicit ClTagsTab(const uint64_t id, QWidget* parent = 0);
};

class ClTagsDialog : public QDialog{
    Q_OBJECT
  public:
    explicit ClTagsDialog(QWidget* parent = 0);
};
#endif
