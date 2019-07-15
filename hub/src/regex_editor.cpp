/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifdef USE_BOOST_REGEX

#include "regex_editor.hpp"

#include "regex_editor_highlighter.hpp"
#include "msgbox.hpp"
#include "sql_name_dialog.hpp"
#include "regex_editor_vars_menu.hpp"

#include <compsky/regex/named_groups.hpp>

#include <boost/regex.hpp>

#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QStringRef>
#include <QHBoxLayout>
#include <QVBoxLayout>


namespace filter_comment_body {
    extern boost::basic_regex<char, boost::cpp_regex_traits<char>>* regexpr;
}

static const QString help_text = 
    "Supports boost::regex Perl syntax, with Python named groups (?P<name>).\n"
    "The group syntax is more flexible than Python's - you can use whatever characters you please, save for '&' and '<', and can use the same group name for multiple groups.\n"
    "Group names are indicated with bold blue text.\n"
    "\n"
    "The first spaces and tabs of each line are ignored, except if the newline was escaped.\n"
    "This is indicated by green highlighting (NOTE: a bug currently means that space after escaped newlines are also (wrongly) highlighted, however this only affects the syntax highlighter and not the pre-processor itself).\n"
    "\n"
    "All text after a (space or tab) followed by '#' is ignored, and all unescaped preceding spaces and tabs too.\n"
    "This is indicated by green highlighting.\n"
    "\n"
    "Unescaped trailing whitespace is NOT ignored; but since it may be accidental, is highlighted cyan.\n"
    "\n"
    "Only \\\\, \\n, \\r, \\t, and \\v are recognised escapes sequences.\n"
    "They are indicated in red.\n"
    "\n"
    "Variable declarations have an almost identical syntax to named groups: {?P<varname>actual string that will be copied}\n"
    "These encompass strings which can then be copy-pasted using an unescaped ${VARNAME}, substituting VARNAME for the exact name of the variable. This will copy everything (aside from the variable name) within the curly braces - for instance, {?P<foobar>hello}${foobar} would result in the string 'hellohello' appearing in the final regex.\n"
    "Such variables can also be declared seperately to the regex file in the 'Vars' menu.\n"
    "Variable declarations must not share names with each other.\n"
;


RegexEditor::RegexEditor(const QString& human_fp,  const QString& raw_fp,  QWidget* parent) : QDialog(parent), f_human_fp(human_fp), f_raw_fp(raw_fp) {
    QVBoxLayout* l = new QVBoxLayout;
    
    this->text_editor = new QPlainTextEdit;

    QFont font;
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    this->text_editor->setFont(font);
    QFontMetrics metrics(font);
    this->text_editor->setTabStopWidth(metrics.width("    "));

    this->load_file();
    l->addWidget(this->text_editor);
    RegexEditorHighlighter* highlighter = new RegexEditorHighlighter(this->text_editor->document());

    this->vars_menu = new RegexEditorVarsMenu(this);

    QHBoxLayout* hbox = new QHBoxLayout;

    QPushButton* help_btn = new QPushButton("Help", this);
    connect(help_btn, &QPushButton::clicked, this, &RegexEditor::display_help);
    hbox->addWidget(help_btn);

    QPushButton* find_btn = new QPushButton("Find", this);
    connect(find_btn, &QPushButton::clicked, this, &RegexEditor::find_text);
    hbox->addWidget(find_btn);

    QPushButton* view_vars_btn = new QPushButton("Vars", this);
    connect(view_vars_btn, &QPushButton::clicked, this->vars_menu, &RegexEditorVarsMenu::show);
    hbox->addWidget(view_vars_btn);

    this->want_optimisations = new QCheckBox("Optimise", this);
    l->addWidget(this->want_optimisations);

    QPushButton* test_btn = new QPushButton("Test", this);
    connect(test_btn, &QPushButton::clicked, this, &RegexEditor::test_regex);
    hbox->addWidget(test_btn);

    QPushButton* save_btn = new QPushButton("Save", this);
    connect(save_btn, &QPushButton::clicked, this, &RegexEditor::save_to_file);
    hbox->addWidget(save_btn);

    l->addLayout(hbox);
    
    this->setLayout(l);
}

void RegexEditor::find_text(){
    SQLNameDialog* dialog = new SQLNameDialog("Find"); // TODO: Have PatternNameDialog, perhaps which SQLNameDialog inherits. SQLNameDialog also needs a case-insensitive option.
    const int rc = dialog->exec();
    const char* patternstr = dialog->get_pattern_str();
    const bool is_pattern = !(patternstr[0] == '=');
    QString qstr = dialog->name_edit->text();
    
    delete dialog;
    
    if (rc != QDialog::Accepted  ||  qstr.isEmpty())
        return;
    
    const int current_pos = this->text_editor->textCursor().position();
    
    if (!is_pattern)
        qstr = QRegularExpression::escape(qstr);
    const QRegularExpression expr(qstr);
    
    QRegularExpressionMatch match = expr.match(this->text_editor->toPlainText(), current_pos);
    
    if (!match.hasMatch()){
        if (current_pos != 0){
            // If no match found, try again from the start of the document
            match = expr.match(this->text_editor->toPlainText(), 0);
        }
        if (!match.hasMatch()){
            QMessageBox::information(this,  "Pattern not found",  qstr);
            return;
        }
    }
    
    QTextCursor cursor = this->text_editor->textCursor();
    cursor.setPosition(match.capturedStart());
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, match.capturedLength());
    this->text_editor->setTextCursor(cursor);
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

bool RegexEditor::to_final_format(const bool optimise,  QString& buf,  int i,  int j,  int last_optimised_group_indx,  int var_depth){ // Use seperate buffer to avoid overwriting text_editor contents
    // WARNING: Does not currently support special encodings, i.e. non-ASCII characters are likely to be mangled.
    // TODO: Add utf8 support.
    QString q = this->text_editor->toPlainText();
    
    int group_start = 0;
    int group_start_offset;
    static std::vector<QStringRef> var_names;
    static std::vector<QStringRef> var_values;
    static std::vector<int> var_depths;
    static std::vector<int> var_starts;
    for(;  i < q.size();  ){
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
                goto goto_RE_tff_cleanup;
            }
            
            buf[j++] = ch;
            ++i;
            continue;
        }
        if (c == QChar('$')  &&  q.at(i+1) == QChar('{')){
            i += 2; // Skip ${
            const int substitute_var_name_start = i;
            while(q.at(i++) != QChar('}'));
            const QStringRef substitute_var_name(&q,  substitute_var_name_start,  i - 1 /* backtrack } */ - substitute_var_name_start);
            QStringRef var = nullptr;
            for (size_t k = var_names.size();  k != 0;  ){
                --k;
                if (var_names[k] != substitute_var_name)
                    continue;
                var = var_values[k];
            }
            if (var == nullptr){
                // Variable of the given name was not declared before
                QString msg = "Previously defined variables:";
                for (size_t k = var_names.size();  k != 0;  ){
                    msg += "\n";
                    msg += var_names[--k];
                }
                MsgBox* msgbox = new MsgBox(
                    this,  
                    "Undeclared variable: " + substitute_var_name + "\nAt line " + QString::number(get_line_n(q, i)),
                    msg
                );
                msgbox->exec();
                goto goto_RE_tff_cleanup;
            }
            buf += var;
            j += var.size();
            continue;
        }
        if (c == QChar('{')){
            if (q.at(i+1) == QChar('?')  &&  q.at(i+2) == QChar('P')  && q.at(i+3) == QChar('<')){
                i += 4;
                const int var_name_start = i;
                while(q.at(i++) != QChar('>'));
                var_names.emplace_back(&q,  var_name_start,  i - 1 /* Backtrack > */ - var_name_start);
                var_depths.push_back(++var_depth);
                var_values.push_back(nullptr);
                var_starts.push_back(j);
                continue;
            }
        }
        if (c == QChar('}')){
            if (var_depth != 0){
                bool placed = false;
                size_t k = var_depths.size();
                while (var_depths[--k] != var_depth); // k kannot be lower than 0
                var_values[k] = QStringRef(&buf,  var_starts[k],  j - var_starts[k]);
                ++i;
                --var_depth;
                continue;
            }
        }
        if (c == QChar('\n')){
            ++i;
            while((i < q.size())  &&  (q.at(i) == QChar(' ')  ||  q.at(i) == QChar('\t')))
                ++i;
            continue;
        }
        if (c == QChar('#')  &&  (i == 0  ||  q.at(i-1) == QChar(' ')  ||  q.at(i-1) == QChar('\t')  ||  q.at(i-1) == QChar('\n'))){
            ++i;
            do {
                // Remove all preceding unescaped whitespace
                --j;
            } while (j >= 0  && buf.at(j) == QChar(' ')  ||  j >= 1  &&  buf.at(j) == QChar('\t')  &&  buf.at(j-1) != QChar('\\'));
            ++j;
            while((i < q.size())  &&  (q.at(i) != QChar('\n')))
                ++i;
            continue;
        }
        if (optimise){
            if (c == QChar('(')  &&  j != last_optimised_group_indx){ // Minimum offset for non-trivial group: (ab|c)
                group_start_offset = 0;
                if (q.at(i+1) == QChar('?')  &&  q.at(i+2) == QChar(':'))
                    group_start_offset += 3;
                else if (q.at(i+1) == QChar('?')  &&  q.at(i+2) == QChar('P')  &&  q.at(i+3) == QChar('<')){
                    group_start_offset += 4;
                    while(q.at(i+group_start_offset) != QChar('>')){
                        ++group_start_offset;
                    }
                    ++group_start_offset;
                } else group_start_offset += 1;
                group_start = j;
            } else if (c == QChar(')')  &&  group_start != 0){
                const int group_start_actual = group_start + group_start_offset;
                const QStringRef group_text(&buf,  group_start_actual,  j - group_start_actual);
                QString group_replacement;
                const QString group_str = group_text.toString();
                QString s = group_str;
                optimise_regex(s, group_replacement);
                buf.replace(group_start_actual,  j - group_start_actual,  group_replacement);
                return to_final_format(optimise,  buf,  i,  group_start_actual + group_replacement.size(),  group_start,  var_depth);
            }
        }
        
        buf[j++] = c;
        
        ++i;
    }
    buf.resize(j); // Strips excess space, as buf is guaranteed to be smaller than q (until variable substitution is implemented)
    
    var_names.clear();
    var_values.clear();
    var_depths.clear();
    var_starts.clear();
    
    return true;

    goto_RE_tff_cleanup:
    var_names.clear();
    var_values.clear();
    var_depths.clear();
    var_starts.clear();
    return false;
}

void RegexEditor::test_regex(){
    QString buf; // Dummy character to create space for 1 char at beginning
    buf.reserve(this->text_editor->toPlainText().size());
    if (!this->to_final_format(this->does_user_want_optimisations(), buf, 0, 0))
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
        MsgBox* msgbox = new MsgBox(this, e.what(), s, 720);
        msgbox->exec();
        //delete filter_comment_body::regexpr; // No need to delete, as object is not created when error is thrown.
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
    QString buf; // Dummy character to create space for 1 char at beginning
    buf.reserve(this->text_editor->toPlainText().size());
    if (!this->to_final_format(this->does_user_want_optimisations(), buf, 0, 0))
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

bool RegexEditor::does_user_want_optimisations() const {
	return this->want_optimisations->isChecked();
}

#endif
