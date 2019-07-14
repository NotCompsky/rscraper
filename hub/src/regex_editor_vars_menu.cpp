/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "regex_editor_vars_menu.hpp"

#include "name_dialog.hpp"

#include <compsky/mysql/query.hpp>
/*
Why store variables in SQL? Laziness. Filesystem storage is ultimately the goal, as I am mulling detaching regex_editor from RScraper. However, it would be just a little bit of a pain to implement (for instance, would need to specify new environmental variable for the directory to save/load variables from).
*/

#include <QCompleter>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


static const QStringList type_id2name = {
	"0:raw_string",
	"1:trie_array", // i.e. a list - not a string
};

namespace _f {
	constexpr static const compsky::asciify::flag::Escape esc;
}


RegexEditorVarsMenu::RegexEditorVarsMenu(QWidget* parent) : QDialog(parent) {
    this->l = new QGridLayout;
    
    {
    QPushButton* btn = new QPushButton("+Var", this);
    connect(btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::add_var);
    this->l->addWidget(btn, 0, 0);
    }

    compsky::mysql::query_buffer(&RES1,  "SELECT name, type FROM regex_vars");
    
    {
    char* name;
    unsigned int type;
    
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &name, &type)){
        add_var_row(name,  type_id2name[type]);
    }
    }
    
    this->setLayout(this->l);
}

void RegexEditorVarsMenu::add_var(){
    bool ok;
    const QString name = QInputDialog::getText(this,  "New Variable",  "Name",  QLineEdit::Normal,  "",  &ok);
    if (!ok || name.isEmpty())
        return;
    /* Using NameDialog for this is silly - a custom dialog (with radio buttons) would be neater */
    NameDialog* dialog = new NameDialog("New Variable: Type", "");
    QCompleter* type_completer = new QCompleter(type_id2name);
    dialog->name_edit->setCompleter(type_completer);
    const int rc = dialog->exec();
    const QString type_name = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted  ||  type_name.isEmpty())
        return;
    const unsigned int type = type_id2name.indexOf(type_name);
    if (type == -1){
        QMessageBox::information(this,  "Invalid type",  "Invalid type");
        return;
    }

    compsky::mysql::exec("INSERT IGNORE INTO regex_vars (name, type) VALUES (\"",  _f::esc, '"', name,  "\",",  type,  ")");
    
    this->add_var_row(name, type_name);
}

void RegexEditorVarsMenu::add_var_row(const QString name,  const QString type_name){
    ++this->row;
    
    this->l->addWidget(new QLabel(name, this),       this->row,  0);
    QLabel* type_label = new QLabel(type_name, this);
    this->l->addWidget(type_label,                   this->row,  1);
    type_label->setObjectName(QString("%1").arg(type_id2name.indexOf(type_name)));
    QPushButton* edit_btn = new QPushButton("Edit", this);
    edit_btn->setObjectName(QString("%1").arg(this->row));
    connect(edit_btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::var_edit_btn_clicked);
    this->l->addWidget(edit_btn,                     this->row,  2);
    QPushButton* proc_btn = new QPushButton("Process", this);
    proc_btn->setObjectName(QString("%1").arg(this->row));
    this->l->addWidget(proc_btn,                     this->row,  3);
    connect(proc_btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::var_proc_btn_clicked);
}

void RegexEditorVarsMenu::var_edit_btn_clicked(){
    QWidget* btn = static_cast<QPushButton*>(sender());
    const int row = btn->objectName().toInt();
    const QString name = static_cast<QLabel*>(this->l->itemAtPosition(row, 0)->widget())->text();

    compsky::mysql::query(&RES1,  "SELECT data FROM regex_vars WHERE name=\"", _f::esc, '"', name, "\"");
    char* data_raw;
    QString data;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &data_raw)){
        static const QString qstr_uninitialised("!!!<Uninitialised>!!!");
        data = (data_raw==nullptr) ? qstr_uninitialised : QString(data_raw);
    }

    bool ok;
    data = QInputDialog::getMultiLineText(this,  "Edit Variable",  "Data",  data,  &ok);
    if (!ok)
        return;

    compsky::mysql::exec("UPDATE regex_vars SET data=\"", _f::esc, '"', data, "\" WHERE name=\"", _f::esc, '"', name, "\"");
}

void RegexEditorVarsMenu::var_proc_btn_clicked(){
    QWidget* btn = static_cast<QPushButton*>(sender());
    const int row = btn->objectName().toInt();
    const QString name = static_cast<QLabel*>(this->l->itemAtPosition(row, 0)->widget())->text();
    const unsigned int type = this->l->itemAtPosition(row, 1)->widget()->objectName().toInt();

    compsky::mysql::query(&RES1,  "SELECT data FROM regex_vars WHERE name=\"", _f::esc, '"', name, "\"");
    char* data;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &data)){
        QString result;
        switch(type){
            case 1: {
                /* Create trie. Each line represents a member. */
                QProcess regtrie;
                QString output;
                QStringList args;
                for (char* itr = data;  *itr != 0;  ++itr){
                    if (*itr == '\n'){
                        *itr = '|';
                    }
                }
                args << data;
                regtrie.start("regopt.pl", args);
                if (!regtrie.waitForFinished()){
                    QMessageBox::warning(this,  "Error",  "Cannot execute regopt.pl");
                    return;
                }
                result = regtrie.readAllStandardOutput();
                regtrie.close();
                break;
            } default:
                result = (data == nullptr) ? "<UNINITIALISED>" : data;
                break;
        }
        QMessageBox::information(this,  "Result",  result);
        /* Save result to database */
    }
}
