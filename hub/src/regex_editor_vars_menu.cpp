/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "regex_editor_vars_menu.hpp"
#include "mysql_declarations.hpp"
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


static const QStringList type_id2name = {
	"0:raw_string",
	"1:array", // i.e. a list - not a string
};


void optimise_regex(QString& data,  QString& result){
	QProcess regtrie;
	QStringList args;
	for (int i = data.length();  i != 0;  ){
		--i;
		if (data[i] == QChar('\n')){
			data[i] = QChar('|');
		}
	}
	args << data;
	regtrie.start("regopt.pl", args);
	if (!regtrie.waitForFinished()){
		QMessageBox::warning(0,  "Error",  "Cannot execute regopt.pl");
		return;
	}
	result = regtrie.readAllStandardOutput();

	/* Code to replace the groups that match start of string '^' with groups that do not */
	int i = 0;
	while(result[i] == QChar('(')  &&  result[i+1] == QChar('?')  &&  result[i+2] == QChar('^')  && result[i+3] == QChar(':')){
		i += 4;
	}
	int j = i / 4;
	for (auto k = j;  k != 0;  --k){
		result[--i] = QChar(':');
		result[--i] = QChar('?');
		result[--i] = QChar('(');
	}
	result.remove(0, j); // Remove (in-place) the now-unused characters

	regtrie.close();
}

RegexEditorVarsMenu::RegexEditorVarsMenu(QWidget* parent) : QDialog(parent), row(0) {
	this->l = new QGridLayout;
	
	{
	QPushButton* btn = new QPushButton("+Var", this);
	connect(btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::add_var);
	this->l->addWidget(btn, 0, 0);
	}

	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1,  "SELECT name, type FROM regex_vars");
	
	{
	const char* name;
	unsigned int type;
	
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &name, &type)){
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

	compsky::mysql::exec(_mysql::obj, BUF, "INSERT IGNORE INTO regex_vars (name, type) VALUES (\"",  _f::esc, '"', name,  "\",",  type,  ")");
	
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
	QPushButton* view_btn = new QPushButton("Preview", this);
	view_btn->setObjectName(QString("%1").arg(this->row));
	this->l->addWidget(view_btn,                     this->row,  3);
	connect(view_btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::var_view_btn_clicked);
	QPushButton* del_btn = new QPushButton("Delete", this);
	del_btn->setObjectName(QString("%1").arg(this->row));
	QPalette palette = del_btn->palette();
	palette.setColor(QPalette::Button, QColor(Qt::red));
	del_btn->setAutoFillBackground(true);
	del_btn->setPalette(palette);
	del_btn->setFlat(true);
	del_btn->update();
	this->l->addWidget(del_btn,                      this->row,  4);
	connect(del_btn, &QPushButton::clicked, this, &RegexEditorVarsMenu::var_del_btn_clicked);
#   define n_per_row 5
}

void RegexEditorVarsMenu::var_edit_btn_clicked(){
	QWidget* btn = static_cast<QPushButton*>(sender());
	const int row_ = btn->objectName().toInt();
	const QString name = static_cast<QLabel*>(this->l->itemAtPosition(row_, 0)->widget())->text();

	compsky::mysql::query(_mysql::obj, _mysql::res1,  BUF, "SELECT data FROM regex_vars WHERE name=\"", _f::esc, '"', name, "\"");
	const char* data_raw;
	QString data_;
	while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &data_raw)){
		static const QString qstr_uninitialised("!!!<Uninitialised>!!!");
		data_ = (data_raw==nullptr) ? qstr_uninitialised : QString(data_raw);
	}

	bool ok;
	data_ = QInputDialog::getMultiLineText(this,  "Edit Variable",  "Data",  data_,  &ok);
	if (!ok)
		return;

	compsky::mysql::exec(_mysql::obj, BUF, "UPDATE regex_vars SET data=\"", _f::esc, '"', data_, "\" WHERE name=\"", _f::esc, '"', name, "\"");

	const unsigned int type = this->l->itemAtPosition(row_, 1)->widget()->objectName().toInt();

	{
		QString result;
		switch(type){
			case 1: {
				optimise_regex(data_, result);
				break;
			} default:
				result = (data_ == nullptr) ? "<UNINITIALISED>" : data_;
				break;
		}
		QMessageBox::information(this,  "Result",  result);
		compsky::mysql::exec(_mysql::obj, BUF, "UPDATE regex_vars SET result=\"", _f::esc, '"', result, "\" WHERE name=\"", name, "\"");
	}
}

void RegexEditorVarsMenu::var_view_btn_clicked(){
	QWidget* btn = static_cast<QPushButton*>(sender());
	const int row_ = btn->objectName().toInt();
	const QString name = static_cast<QLabel*>(this->l->itemAtPosition(row_, 0)->widget())->text();
	const unsigned int type = this->l->itemAtPosition(row_, 1)->widget()->objectName().toInt();

	compsky::mysql::query(_mysql::obj, _mysql::res1, BUF,  "SELECT result FROM regex_vars WHERE name=\"", _f::esc, '"', name, "\"");
	const char* result;
	while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &result)){
		QMessageBox::information(this,  "Result",  result);
	}
}

void RegexEditorVarsMenu::var_del_btn_clicked(){
	QWidget* btn = static_cast<QPushButton*>(sender());
	const int row_ = btn->objectName().toInt();
	const QString name = static_cast<QLabel*>(this->l->itemAtPosition(row_, 0)->widget())->text();
	compsky::mysql::exec(_mysql::obj, BUF, "DELETE FROM regex_vars WHERE name=\"", _f::esc, '"', name, "\"");
	for (auto i = 0;  i < n_per_row;  ++i){
		QLayoutItem* a = this->l->itemAtPosition(row_, i);
		delete a->widget();
		l->removeItem(a);
	}
}
