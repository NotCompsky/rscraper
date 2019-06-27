/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "scraper_tab.hpp"

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <QCompleter>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

#include "id2str.hpp"

#include "categorytab.hpp"
#include "name_dialog.hpp"
#include "notfound.hpp"
#ifdef USE_BOOST_REGEX
# include "regex_editor.hpp"
#endif
#include "wlbl_label.hpp"
#include "wlbl_reasonwise_label.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QStringList subreddit_names;
extern QCompleter* subreddit_name_completer;
QStringList user_names;
QCompleter* user_name_completer = nullptr;
QStringList reason_names;
QCompleter* reason_name_completer;


namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}


void populate_user_name_completer(){
    compsky::mysql::query_buffer(&RES1, "SELECT name FROM user");
    char* name;
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
        user_names << name;
    }
    user_name_completer = new QCompleter(user_names);
}

void populate_reason_name_completer(){
    compsky::mysql::query_buffer(&RES1, "SELECT name FROM reason_matched");
    char* name;
    while (compsky::mysql::assign_next_row(RES1, &ROW1, &name)){
        reason_names << name;
    }
    reason_name_completer = new QCompleter(reason_names);
}


int ScraperTab::add(ScraperTabMemberFnct f_add,  ScraperTabMemberFnct f_rm,  QGridLayout* l,  int row){
    {
    QPushButton* btn1 = new QPushButton("+", this);
    connect(btn1, &QPushButton::clicked, this, f_add);
    l->addWidget(btn1, row, 1);
    QPushButton* btn2 = new QPushButton("-", this);
    connect(btn2, &QPushButton::clicked, this, f_rm);
    l->addWidget(btn2, row, 2);
    ++row;
    }
    
    return row;
}


ScraperTab::ScraperTab(QWidget* parent) : QWidget(parent) {
    QGridLayout* l = new QGridLayout;
    
    int row = 0;
    
    
    populate_reason_name_completer();
    
    
    l->addWidget(new QLabel("Changes are only enacted the next time rscrape-cmnts starts"), row++, 0);
    
  #ifdef USE_BOOST_REGEX
    QPushButton* edit_cmnt_body_re_btn = new QPushButton("Edit Comment Body Regexp", this);
    connect(edit_cmnt_body_re_btn, &QPushButton::clicked, this, &ScraperTab::open_cmnt_body_re_editor);
    l->addWidget(edit_cmnt_body_re_btn, row++, 0);
  #endif
    
    l->addWidget(new QLabel("Count comments in subreddits"), row++, 0);
    l->addWidget(new WlBlLabel("Blacklist", "subreddit", "id", "subreddit_count_bl"),  row,  0);
    row = add(
        &ScraperTab::add_to_subreddit_count_bl,
        &ScraperTab::rm_from_subreddit_count_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Count comments by users"), row++, 0);
    l->addWidget(new WlBlLabel("Blacklist", "user", "id", "user_count_bl"),  row,  0);
    row = add(
        &ScraperTab::add_to_user_count_bl,
        &ScraperTab::rm_from_user_count_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Record comment contents in subreddits"), row++, 0);
    l->addWidget(new WlBlLabel("Whitelist", "subreddit", "id", "subreddit_contents_wl"),  row,  0);
    row = add(
        &ScraperTab::add_to_subreddit_contents_wl,
        &ScraperTab::rm_from_subreddit_contents_wl,
        l,
        row
    );
    l->addWidget(new WlBlLabel("Blacklist", "subreddit", "id", "subreddit_contents_bl"),  row,  0);
    row = add(
        &ScraperTab::add_to_subreddit_contents_bl,
        &ScraperTab::rm_from_subreddit_contents_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Record comment contents by users"), row++, 0);
    l->addWidget(new WlBlLabel("Whitelist", "user", "id", "user_contents_wl"),  row,  0);
    row = add(
        &ScraperTab::add_to_user_contents_wl,
        &ScraperTab::rm_from_user_contents_wl,
        l,
        row
    );
    l->addWidget(new WlBlLabel("Blacklist", "user", "id", "user_contents_bl"),  row,  0);
    row = add(
        &ScraperTab::add_to_user_contents_bl,
        &ScraperTab::rm_from_user_contents_bl,
        l,
        row
    );
    
    /* TODO: On right click, display reason vs subreddit, rather than just subreddit */
    l->addWidget(new QLabel("Comment content filters (on a per-reason basis)"), row++, 0);
    l->addWidget(new WlBlReasonwiseLabel("Subreddit Whitelists", "subreddit", "subreddit", "reason_subreddit_whitelist"),  row,  0); // third argument is shorthand for subreddit_id
    row = add(
        &ScraperTab::add_to_reason_subreddit_wl,
        &ScraperTab::rm_from_reason_subreddit_wl,
        l,
        row
    );
    l->addWidget(new WlBlReasonwiseLabel("Subreddit Blacklists", "subreddit", "subreddit", "reason_subreddit_blacklist"),  row,  0); // third argument is shorthand for subreddit_id
    row = add(
        &ScraperTab::add_to_reason_subreddit_bl,
        &ScraperTab::rm_from_reason_subreddit_bl,
        l,
        row
    );
    
    
    setLayout(l);
}

#ifdef USE_BOOST_REGEX
void ScraperTab::open_cmnt_body_re_editor(){
    QString qfp = getenv("RSCRAPER_REGEX_FILE");
    if (qfp == nullptr){
        QMessageBox::information(this, "Invalid Action", "Environmental variable RSCRAPER_REGEX_FILE must be set to the file to save this regex to");
        return;
    }
    
    RegexEditor* editor = new RegexEditor(qfp + ".human",  qfp,  this);
    editor->exec();
}
#endif


void ScraperTab::add_subreddit_to(const char* tblname,  const bool delete_from){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "", "Use SQL LIKE pattern matching");
    if (!delete_from)
        dialog->name_edit->setCompleter(subreddit_name_completer);
    else {
        QStringList tbl_user_names;
        compsky::mysql::query(&RES1, "SELECT s.name, t.id FROM subreddit s RIGHT JOIN ", tblname, " t ON s.id=t.id");
        char* name;
        uint64_t id;
        char buf[20];
        while(compsky::mysql::assign_next_row(RES1, &ROW1, &name, &id)){
            if (name != nullptr)
                tbl_user_names << name;
            else {
                buf[id2str(id, buf)] = 0;
                tbl_user_names << QString("id-t3_") + buf;
            }
        }
        dialog->name_edit->setCompleter(new QCompleter(tbl_user_names));
    }
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    const bool is_pattern = dialog->checkbox->isChecked();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    if (!is_pattern  &&  !subreddit_names.contains(qstr))
        return notfound::subreddit(this, qstr);
    
    compsky::mysql::exec(
        (delete_from) ? "DELETE a FROM " : "INSERT IGNORE INTO ",
        tblname,
        (delete_from) ? " a, subreddit b WHERE a.id=b.id AND b.name=\"" : " SELECT id FROM subreddit WHERE name",
        (is_pattern) ? " LIKE \"" : "=\"",
            qstr,
        "\""
    );
}

void ScraperTab::add_subreddit_to_reason(const char* tblname,  const bool delete_from){
    bool ok;
    NameDialog* dialog;
    int rc;
    
    dialog = new NameDialog("Reason", "");
    dialog->name_edit->setCompleter(reason_name_completer);
    rc = dialog->exec();
    const QString qstr_reason = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_reason.isEmpty())
        return;
    
    if (!reason_names.contains(qstr_reason))
        return notfound::reason(this, qstr_reason);
    
    dialog = new NameDialog("Subreddit", "", "Use SQL LIKE pattern matching");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    rc = dialog->exec();
    const QString qstr_subreddit = dialog->name_edit->text();
    const bool is_pattern = dialog->checkbox->isChecked();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_subreddit.isEmpty())
        return;
    
    if (!is_pattern  &&  !subreddit_names.contains(qstr_subreddit))
        return notfound::subreddit(this, qstr_subreddit);
    
    compsky::mysql::exec(
        (delete_from) ? "DELETE x FROM " : "INSERT IGNORE INTO ",
        tblname,
        (delete_from) ? " x, reason_matched a, subreddit b WHERE x.reason=a.id AND x.subreddit=b.id AND a.name=\"" : " SELECT a.id,b.id FROM reason_matched a, subreddit b WHERE a.name=\"",
        qstr_reason,
        "\" AND b.name",
        (is_pattern) ? " LIKE " : "=",
        '"',
        qstr_subreddit,
        '"'
    );
}

void ScraperTab::add_user_to(const char* tblname,  const bool delete_from){
    bool ok;
    
    if (user_name_completer == nullptr)
        populate_user_name_completer();
    
    NameDialog* dialog = new NameDialog(tblname, "", "Is user ID (format id-t2_a1b2c3d)");
    if (!delete_from)
        dialog->name_edit->setCompleter(user_name_completer);
    else {
        QStringList tbl_user_names;
        compsky::mysql::query(&RES1, "SELECT u.name, t.id FROM user u RIGHT JOIN ", tblname, " t ON u.id=t.id");
        char* name;
        uint64_t id;
        char buf[20];
        while(compsky::mysql::assign_next_row(RES1, &ROW1, &name, &id)){
            if (name != nullptr)
                tbl_user_names << name;
            else {
                buf[id2str(id, buf)] = 0;
                tbl_user_names << QString("id-t2_") + buf;
            }
        }
        dialog->name_edit->setCompleter(new QCompleter(tbl_user_names));
    }
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    const bool is_id = dialog->checkbox->isChecked();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    if (is_id){
        QByteArray ba = qstr.toLocal8Bit();
        const char* s = ba.data();
        if (s[0] != 'i'  ||  s[1] != 'd'  ||  s[2] != '-'  ||  s[3] != 't'  ||  s[4] != '2'  ||  s[5] != '_'){
            QMessageBox::information(this, "Invalid Format", "User ID must be in format id-t2_<alphanumerics>");
            return;
        }
        compsky::mysql::exec(
            (delete_from) ? "DELETE FROM " : "INSERT IGNORE INTO ",
            tblname,
            (delete_from) ? " WHERE id=" : " (id) VALUES (", str2id(s + 6), ")"
        );
        return;
    }
    
    if (!user_names.contains(qstr))
        return notfound::user(this, qstr);
    
    compsky::mysql::exec(
        (delete_from) ? "DELETE a FROM " : "INSERT IGNORE INTO ",
        tblname,
        (delete_from) ? "a, user b WHERE a.id=b.id AND b.name=\"" : " SELECT id FROM user WHERE name=\"",
            // NOTE: No need to escape '"' in qstr - qstr must be a valid username, and valid usernames do not contain '"'
            qstr,
        "\""
    );
}


void ScraperTab::add_to_subreddit_count_bl(){
    this->add_subreddit_to("subreddit_count_bl", false);
}

void ScraperTab::rm_from_subreddit_count_bl(){
    this->add_subreddit_to("subreddit_count_bl", true);
}


void ScraperTab::add_to_user_count_bl(){
    this->add_user_to("user_count_bl", false);
}

void ScraperTab::rm_from_user_count_bl(){
    this->add_user_to("user_count_bl", true);
}


void ScraperTab::add_to_subreddit_contents_wl(){
    this->add_subreddit_to("subreddit_contents_wl", false);
}

void ScraperTab::rm_from_subreddit_contents_wl(){
    this->add_subreddit_to("subreddit_contents_wl", true);
}


void ScraperTab::add_to_subreddit_contents_bl(){
    this->add_subreddit_to("subreddit_contents_bl", false);
}

void ScraperTab::rm_from_subreddit_contents_bl(){
    this->add_subreddit_to("subreddit_contents_bl", true);
}


void ScraperTab::add_to_user_contents_wl(){
    this->add_user_to("user_contents_wl", false);
}

void ScraperTab::rm_from_user_contents_wl(){
    this->add_user_to("user_contents_wl", true);
}


void ScraperTab::add_to_user_contents_bl(){
    this->add_user_to("user_contents_bl", false);
}

void ScraperTab::rm_from_user_contents_bl(){
    this->add_user_to("user_contents_bl", true);
}


void ScraperTab::add_to_reason_subreddit_wl(){
    this->add_subreddit_to_reason("reason_subreddit_whitelist", false);
}

void ScraperTab::rm_from_reason_subreddit_wl(){
    this->add_subreddit_to_reason("reason_subreddit_whitelist", true);
}


void ScraperTab::add_to_reason_subreddit_bl(){
    this->add_subreddit_to_reason("reason_subreddit_blacklist", false);
}

void ScraperTab::rm_from_reason_subreddit_bl(){
    this->add_subreddit_to_reason("reason_subreddit_blacklist", true);
}


