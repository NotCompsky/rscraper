#ifndef CREATE_CHAR_WINDOW_H
#define CREATE_CHAR_WINDOW_H

#include <QTabWidget>
#include <QDialogButtonBox>
#include <QDialog>

#include <inttypes.h> // for uintN_t


struct tag2cl{
    uint64_t tagid;
    char* tagname;
    uint32_t cl;
};




class ClTagsDialog : public QDialog{
    Q_OBJECT
  public:
    ~ClTagsDialog();
    explicit ClTagsDialog(QWidget* parent = 0);
};
#endif
