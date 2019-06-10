#include "tagdialog.hpp"

#include <QCompleter>
#include <QStringList>
#include <QTimer>


extern QStringList tagslist;


TagDialog::TagDialog(QString title,  QString str,  QWidget* parent) : QDialog(parent){
    // If the functions are implemented in the header file you have to declare the definitions of the functions with inline to prevent having multiple definitions of the functions.
    this->btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(this->btn_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this->btn_box, SIGNAL(rejected()), this, SLOT(reject()));
    l = new QVBoxLayout;
    l->addWidget(this->btn_box);
    this->name_edit = new QLineEdit(str);
    QCompleter* tagcompleter = new QCompleter(tagslist);
    this->name_edit->setCompleter(tagcompleter);
    l->addWidget(this->name_edit);
    this->setLayout(l);
    this->setWindowTitle(title);
    QTimer::singleShot(0, this->name_edit, SLOT(setFocus())); // Set focus after TagDialog instance is visible
}

TagDialog::~TagDialog(){
    delete this->name_edit;
    delete this->l;
    delete this->btn_box;
}
