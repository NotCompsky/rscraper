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

const char* RegexEditor::to_final_format(){
    // WARNING: Does not currently support special encodings, i.e. non-ASCII characters are likely to be mangled.
    // TODO: Add utf8 support.
    const QString q = this->text_editor->toPlainText();
    const QByteArray b = q.toLocal8Bit();
    const char* orig = b.data();
    char* buf = (char*)malloc(b.size() + 1); // WARNING: QByteArray::size() is only an int
    if (buf == nullptr){
        QMessageBox::critical(this, "Memory Error",  "Cannot allocate memory",  QMessageBox::Cancel);
        return "";
    }
    memcpy(buf, orig, b.size());
    buf[b.size()] = 0;
    
    char* const start = buf;
    char* s = buf;
    
    while(*s != 0){
        if (*s == '\\'){
            // Recognised escapes: \n, \r, \t, \v
            // All others simply become the literal value of the next character
            char c = *(++s);
            if      (c == 0)
                break;
            else if (c == 'n') c = '\n';
            else if (c == 'r') c = '\r';
            else if (c == 't') c = '\t';
            else if (c == 'v') c = '\v'; // Vertical tab
            *(buf++) = c;
            ++s;
            continue;
        }
        if (*s == '\n'){
            ++s;
            while((*s != 0)  &&  (*s == ' '  ||  *s == '\t'))
                ++s;
            continue;
        }
        if (*s == '#'){
            ++s;
            do {
                // Remove all preceding unescaped whitespace
                --buf;
            } while ((*buf == ' '  ||  *buf == '\t')  &&  (*(buf-1) != '\\'));
            ++buf;
            while((*s != 0)  &&  (*s != '\n'))
                ++s;
            continue;
        }
        
        *(buf++) = *s;
        
        ++s;
    }
    *buf = 0;
    
    return start;
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
    s_raw   << this->to_final_format();
    s_raw.flush();
    
    this->close(); // Avoids issues with closing and reopening QFiles
}
