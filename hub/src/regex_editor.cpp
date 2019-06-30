/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifdef USE_BOOST_REGEX

#include "regex_editor.hpp"

#include <boost/regex.hpp>

#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStringRef>
#include <QVBoxLayout>

#include <compsky/regex/named_groups.hpp>

#include "msgbox.hpp"


namespace filter_comment_body {
    extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;
}

static const QString help_text = 
    "Supports boost::regex Perl syntax, with Python named groups (?P<name>).\n"
    "The first spaces and tabs of each line are ignored, except if the newline was escaped.\n"
    "All text after an unescaped # is ignored, and all unescaped preceding spaces and tabs too.\n"
    "Only \\\\, \\n, \\r, \\t, and \\v are recognised escapes sequences.\n"
;


RegexEditor::RegexEditor(const QString& human_fp,  const QString& raw_fp,  QWidget* parent) : QDialog(parent), f_human_fp(human_fp), f_raw_fp(raw_fp) {
    QGridLayout* l = new QGridLayout;
    
    QPushButton* help_btn = new QPushButton("Help", this);
    connect(help_btn, &QPushButton::clicked, this, &RegexEditor::display_help);
    l->addWidget(help_btn);
    
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

void RegexEditor::display_help(){
    QMessageBox::information(this,  "Help",  help_text);
}

void RegexEditor::load_file(){
    QFile f_human(this->f_human_fp);
    if (!f_human.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::critical(this, "FS Error",  "Cannot read: " + this->f_human_fp);
        this->text_editor->setReadOnly(true);
        return;
    }
    this->text_editor->setPlainText(f_human.readAll());
    f_human.close();
}

int get_line_n(QString& s,  int end){
    int n = 0;
    for (auto i = 0;  i <= end;  ++i)
        if (s.at(i) == QChar('\n'))
            ++n;
    return n;
}

bool RegexEditor::to_final_format(QString& buf,  int j){ // Use seperate buffer to avoid overwriting text_editor contents
    // WARNING: Does not currently support special encodings, i.e. non-ASCII characters are likely to be mangled.
    // TODO: Add utf8 support.
    QString q = this->text_editor->toPlainText();
    buf.reserve(q.size() + j);
    
    for(auto i = 0;  i < q.size();  ){
        QChar c = q.at(i);
        if (c == QChar('\\')){
            // Recognised escapes: \\, \n, \r, \t, \v
            // All others simply become the literal value of the next character
            if (++i == q.size())
                break;
            QChar ch = q.at(i);
            if      (ch == QChar('n'))  ch = QChar('\n');
            else if (ch == QChar('r'))  ch = QChar('\r');
            else if (ch == QChar('t'))  ch = QChar('\t');
            else if (ch == QChar('v'))  ch = QChar('\v'); // Vertical tab
            else if (ch == QChar('\\'));
            else if (ch == QChar('\n'));
            else if (ch == QChar('\t'));
            else if (ch == QChar(' '));
            else {
                constexpr static const int ctx = 10;
                MsgBox* msgbox = new MsgBox(
                    this,  
                    "Unrecognised escape sequence: \\" + QString(ch) + " at line " + QString::number(get_line_n(q, i)),
                    QStringRef(&q,  (i >= ctx) ? i - ctx : 0,  (i + ctx < q.size()) ? i + ctx : q.size() - 1).toString()
                );
                msgbox->exec();
                return false;
            }
            
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
    
    return true;
}

void RegexEditor::test_regex(){
    QString buf = "_"; // Dummy character to create space for 1 char at beginning
    if (!this->to_final_format(buf, 1))
        return;
    
    QByteArray ba = buf.toLocal8Bit();
    char* const s = ba.data();
    
    std::vector<char*> reason_name2id = {"None", "Unspecified"};
    std::vector<int> groupindx2reason;
    std::vector<char*> group_starts;
    std::vector<char*> group_ends;
    std::vector<bool> record_contents;
    
    char* regexpr_str_end = compsky::regex::convert_named_groups(s + 1,  s,  reason_name2id,  groupindx2reason, record_contents, group_starts, group_ends);
    // Add one to the first buffer (src) not second buffer (dst) to ensure it is never overwritten when writing dst
    
    if (*(regexpr_str_end - 1) == '\n')
        // Very confused what is happening here, but it seems that some files have \n appended to them by fread, while others do not.
        // A small test file containing only `(?P<test>a)` written in `vim` is given a trailing \n by fread
        // while a larger file containing newlines elsewhere but not at the end is not given a trailing \n by fread
        *(regexpr_str_end - 1) = 0;
    else *regexpr_str_end = 0;
    
    try {
        if (filter_comment_body::regexpr != nullptr)
            delete filter_comment_body::regexpr;
        filter_comment_body::regexpr = new boost::basic_regex<char, boost::cpp_regex_traits<char>>(s,  boost::regex::perl);
    } catch (boost::regex_error& e){
        QMessageBox::critical(this,  "Bad Regex",  e.what());
        delete filter_comment_body::regexpr;
        filter_comment_body::regexpr = nullptr;
        return;
    }
    
    QString report = "";
    
    bool try_exrex = true;
    report += "\nCapture Groups:";
    for (auto i = 1;  i < groupindx2reason.size();  ++i){
        report += "\n";
        report += QString::number(i);
        report += "\t";
        report += (record_contents[i]) ? "[Record contents]" : "[Count occurances]";
        report += "\t";
        report += reason_name2id[groupindx2reason[i]];
        report += "\n\t";
        const int group_source_strlen = (uintptr_t)(group_ends[i]) - (uintptr_t)(group_starts[i]) - 1;
        const QString group_source = QString::fromLocal8Bit(group_starts[i],  group_source_strlen);
        if (group_source_strlen > 20){
            report += QString::fromLocal8Bit(group_starts[i], 20);
            report += "...";
        } else report += group_source;
        
        if (try_exrex){
            QProcess exrex;
            QString output;
            
            exrex.start("exrex.py",  {"-r", "-m", "1", group_source});
            
            if (!(try_exrex = exrex.waitForFinished()))
                continue;
            
            report += "\neg:\t";
            report += exrex.readAllStandardOutput();
            
            exrex.close();
        }
    }
    
    if (!try_exrex)
        report += "\n\nTo see example strings that match each group regex, pip install exrex";
    
    MsgBox* msgbox = new MsgBox(this, "Success", report, 720);
    msgbox->exec();
}

void RegexEditor::save_to_file(){
    QString buf;
    if (!this->to_final_format(buf))
        return;
    
    QFile f_raw(this->f_raw_fp);
    if (!f_raw.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::critical(this, "FS Error",  "Cannot write to: " + this->f_raw_fp + "\n" + f_raw.errorString());
        return;
    }
    f_raw.write(buf.toLocal8Bit());
    f_raw.close();
    
    QFile f_human(this->f_human_fp);
    if (!f_human.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::critical(this, "FS Error",  "Cannot write to: " + this->f_human_fp + "\n" + f_human.errorString());
        return;
    }
    f_human.write(this->text_editor->toPlainText().toLocal8Bit());
    f_human.close();
    
    this->close(); // Avoids issues with closing and reopening QFiles
}

#endif
