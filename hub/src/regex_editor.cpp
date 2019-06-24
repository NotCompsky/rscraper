/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "regex_editor.hpp"

#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QTextStream>
#include <QVBoxLayout>


RegexEditor::RegexEditor(const QString& human_fp,  const QString& raw_fp,  QWidget* parent) : QDialog(parent), f_human(human_fp), f_raw(raw_fp) {
    QGridLayout* l = new QGridLayout;
    
    this->text_editor = new QPlainTextEdit;
    this->load_file();
    l->addWidget(this->text_editor);
    
    QPushButton* test_btn = new QPushButton("Test", this);
    connect(test_btn, &QPushButton::clicked, this, &RegexEditor::test_regex);
    l->addWidget(test_btn);
    
    QPushButton* save_btn = new QPushButton("Save", this);
    connect(save_btn, &QPushButton::clicked, this, &RegexEditor::save_to_file);
    l->addWidget(save_btn);
    
    this->setLayout(l);
}

void RegexEditor::load_file(){
    if (!this->f_human.open(QFile::ReadWrite | QFile::Text)  ||  !this->f_raw.open(QFile::WriteOnly | QFile::Text)){
        // Attempt to open the files before opening for editing, rather than only when saving, because it would be incredibly annoying to have edited the regex only to be unable to save it.
        QMessageBox::critical(this, "FS Error",  "Cannot open files",  QMessageBox::Cancel);
        this->text_editor->setReadOnly(true);
        return;
    }
    this->text_editor->setPlainText(this->f_human.readAll());
}

const QString RegexEditor::to_final_format(){
    // WARNING: Does not currently support special encodings, i.e. non-ASCII characters are likely to be mangled.
    // TODO: Add utf8 support.
    QString q = this->text_editor->toPlainText();
    QString buf; // Do not want to overwrite text_editor contents
    buf.reserve(q.size());
    auto j = 0;
    
    for(auto i = 0;  i < q.size();  ){
        QChar c = q.at(i);
        if (c == QChar('\\')){
            // Recognised escapes: \n, \r, \t, \v
            // All others simply become the literal value of the next character
            if (++i == q.size())
                break;
            QChar ch = q.at(i);
            if      (ch == QChar('n')) ch = QChar('\n');
            else if (ch == QChar('r')) ch = QChar('\r');
            else if (ch == QChar('t')) ch = QChar('\t');
            else if (ch == QChar('v')) ch = QChar('\v'); // Vertical tab
            buf[j++] = ch;
            ++i;
            continue;
        }
        if (c == QChar('\n')){
            ++i;
            while((i < q.size())  &&  (q.at(i) == QChar(' ')  ||  q.at(i) == QChar('\t')))
                ++i;
            continue;
        }
        if (c == QChar('#')){
            ++i;
            do {
                // Remove all preceding unescaped whitespace
                --j;
            } while ((buf.at(j) == QChar(' ')  ||  buf.at(j) == QChar('\t'))  &&  (buf.at(j-1) != QChar('\\')));
            ++j;
            while((i < q.size())  &&  (q.at(i) != QChar('\n')))
                ++i;
            continue;
        }
        
        buf[j++] = q.at(i);
        
        ++i;
    }
    buf.resize(j);
    
    return buf;
}

void RegexEditor::test_regex(){
    
}

void RegexEditor::save_to_file(){
    if (!this->f_human.resize(0)){ // Wipe contents; otherwise it is appended (due to ReadWrite flag?)
        QMessageBox::critical(this, "FS Error",  "Cannot flush file before saving",  QMessageBox::Cancel);
        return;
    }
    QTextStream s_human(&this->f_human);
    s_human << this->text_editor->toPlainText().toLocal8Bit();
    s_human.flush();
    QTextStream   s_raw(&this->f_raw);
    s_raw   << this->to_final_format().toLocal8Bit();
    s_raw.flush();
    
    this->close(); // Avoids issues with closing and reopening QFiles
}
