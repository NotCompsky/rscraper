/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "maintab.hpp"

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <QCompleter>
#include <QLabel>
#include <QPushButton>

#include "categorytab.hpp"
#include "name_dialog.hpp"
#include "wlbl_label.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QCompleter* subreddit_name_completer;
QStringList user_names;
QCompleter* user_name_completer = nullptr;
QStringList reason_names;
QCompleter* reason_name_completer = nullptr;


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


int MainTab::add(const char* title,  const char* typ,  const char* typ_id_varname,  const char* tblname,  MainTabMemberFnct f_add,  MainTabMemberFnct f_rm,  QGridLayout* l,  int row){
    l->addWidget(new WlBlLabel(title, typ, typ_id_varname, tblname),  row,  0);
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


MainTab::MainTab(QTabWidget* tab_widget,  QWidget* parent) : QWidget(parent), tab_widget(tab_widget) {
    QGridLayout* l = new QGridLayout;
    
    int row = 0;
    
    
    QPushButton* add_tag_btn = new QPushButton("+Category", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_category()));
    l->addWidget(add_tag_btn, row++, 0);
    
    l->addWidget(new QLabel("Count comments in subreddits"), row++, 0);
    row = add(
        "Blacklist",
        "subreddit",
        "id",
        "subreddit_count_bl",
        &MainTab::add_to_subreddit_count_bl,
        &MainTab::rm_from_subreddit_count_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Count comments by users"), row++, 0);
    row = add(
        "Blacklist",
        "user",
        "id",
        "user_count_bl",
        &MainTab::add_to_user_count_bl,
        &MainTab::rm_from_user_count_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Record comment contents in subreddits"), row++, 0);
    row = add(
        "Whitelist",
        "subreddit",
        "id",
        "subreddit_contents_wl",
        &MainTab::add_to_subreddit_contents_wl,
        &MainTab::rm_from_subreddit_contents_wl,
        l,
        row
    );
    row = add(
        "Blacklist",
        "subreddit",
        "id",
        "subreddit_contents_bl",
        &MainTab::add_to_subreddit_contents_bl,
        &MainTab::rm_from_subreddit_contents_bl,
        l,
        row
    );
    l->addWidget(new QLabel("Record comment contents by users"), row++, 0);
    row = add(
        "Whitelist",
        "user",
        "id",
        "user_contents_wl",
        &MainTab::add_to_user_contents_wl,
        &MainTab::rm_from_user_contents_wl,
        l,
        row
    );
    row = add(
        "Blacklist",
        "user",
        "id",
        "user_contents_bl",
        &MainTab::add_to_user_contents_bl,
        &MainTab::rm_from_user_contents_bl,
        l,
        row
    );
    
    /* TODO: On right click, display reason vs subreddit, rather than just subreddit */
    l->addWidget(new QLabel("Comment content filters (on a per-reason basis)"), row++, 0);
    row = add(
        "Subreddit Whitelist",
        "subreddit",
        "subreddit", // shorthand for subreddit_id
        "reason_subreddit_whitelist",
        &MainTab::add_to_reason_subreddit_wl,
        &MainTab::rm_from_reason_subreddit_wl,
        l,
        row
    );
    row = add(
        "Subreddit Blacklists",
        "subreddit",
        "subreddit", // shorthand for subreddit_id
        "reason_subreddit_blacklist",
        &MainTab::add_to_reason_subreddit_bl,
        &MainTab::rm_from_reason_subreddit_bl,
        l,
        row
    );
    
    
    setLayout(l);
}

void MainTab::add_category(){
    bool ok;
    NameDialog* catdialog = new NameDialog("New Category", "");
    if (catdialog->exec() != QDialog::Accepted)
        return;
    const QString cat_qstr = catdialog->name_edit->text();
    if (cat_qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT INTO category (name) VALUES (\"", _f::esc, '"', cat_qstr, "\")");
    
    compsky::mysql::query(&RES1, "SELECT id FROM category WHERE name=\"", _f::esc, '"', cat_qstr, "\"");
    
    uint64_t cat_id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &cat_id));
    
    this->tab_widget->addTab(new ClTagsTab(cat_id, this->tab_widget), cat_qstr);
}


void MainTab::add_subreddit_to(const char* tblname){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT IGNORE INTO ", tblname, " SELECT id FROM subreddit WHERE name=\"", qstr, "\"");
}

void MainTab::rm_subreddit_from(const char* tblname){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("DELETE a FROM ", tblname, " a, subreddit b WHERE a.id=b.id AND b.name=\"", qstr, "\"");
}

void MainTab::add_subreddit_to_reason(const char* tblname){
    bool ok;
    NameDialog* dialog;
    int rc;
    
    dialog = new NameDialog("Reason", "");
    if (user_name_completer == nullptr)
        populate_reason_name_completer();
    dialog->name_edit->setCompleter(reason_name_completer);
    rc = dialog->exec();
    const QString qstr_reason = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_reason.isEmpty())
        return;
    
    dialog = new NameDialog("Subreddit", "");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    rc = dialog->exec();
    const QString qstr_subreddit = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_subreddit.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT IGNORE INTO ", tblname, " SELECT a.id,b.id FROM reason_matched a, subreddit b WHERE a.name=\"", qstr_reason, "\" AND b.name=\"", qstr_subreddit, "\"");
}

void MainTab::rm_subreddit_from_reason(const char* tblname){
    bool ok;
    NameDialog* dialog;
    int rc;
    
    dialog = new NameDialog("Reason", "");
    if (user_name_completer == nullptr)
        populate_reason_name_completer();
    dialog->name_edit->setCompleter(reason_name_completer);
    rc = dialog->exec();
    const QString qstr_reason = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_reason.isEmpty())
        return;
    
    dialog = new NameDialog("Subreddit", "");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    rc = dialog->exec();
    const QString qstr_subreddit = dialog->name_edit->text();
    delete dialog;
    if (rc != QDialog::Accepted)
        return;
    if (qstr_subreddit.isEmpty())
        return;
    
    compsky::mysql::exec("DELETE x FROM ", tblname, "x, reason_matched a, subreddit b WHERE x.reason=a.id AND x.subreddit=b.id AND a.name=\"", qstr_reason, "\" AND b.name=\"", qstr_subreddit, "\"");
}

void MainTab::add_user_to(const char* tblname){
    bool ok;
    
    if (user_name_completer == nullptr)
        populate_user_name_completer();
    
    NameDialog* dialog = new NameDialog(tblname, "");
    dialog->name_edit->setCompleter(user_name_completer);
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT IGNORE INTO ", tblname, " SELECT id FROM user WHERE name=\"", qstr, "\"");
}

void MainTab::rm_user_from(const char* tblname){
    bool ok;
    
    if (user_name_completer == nullptr)
        populate_user_name_completer();
    
    NameDialog* dialog = new NameDialog(tblname, "");
    dialog->name_edit->setCompleter(user_name_completer);
    const auto rc = dialog->exec();
    const QString qstr = dialog->name_edit->text();
    delete dialog;
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("DELETE a FROM ", tblname, " a, user b WHERE a.id=b.id AND b.name=\"", qstr, "\"");
}


void MainTab::add_to_subreddit_count_bl(){
    this->add_subreddit_to("subreddit_count_bl");
}

void MainTab::rm_from_subreddit_count_bl(){
    this->rm_subreddit_from("subreddit_count_bl");
}


void MainTab::add_to_user_count_bl(){
    this->add_user_to("user_count_bl");
}

void MainTab::rm_from_user_count_bl(){
    this->rm_user_from("user_count_bl");
}


void MainTab::add_to_subreddit_contents_wl(){
    this->add_subreddit_to("subreddit_contents_wl");
}

void MainTab::rm_from_subreddit_contents_wl(){
    this->rm_subreddit_from("subreddit_contents_wl");
}


void MainTab::add_to_subreddit_contents_bl(){
    this->add_subreddit_to("subreddit_contents_bl");
}

void MainTab::rm_from_subreddit_contents_bl(){
    this->rm_subreddit_from("subreddit_contents_bl");
}


void MainTab::add_to_user_contents_wl(){
    this->add_user_to("user_contents_wl");
}

void MainTab::rm_from_user_contents_wl(){
    this->rm_user_from("user_contents_wl");
}


void MainTab::add_to_user_contents_bl(){
    this->add_user_to("user_contents_bl");
}

void MainTab::rm_from_user_contents_bl(){
    this->rm_user_from("user_contents_bl");
}


void MainTab::add_to_reason_subreddit_wl(){
    this->add_subreddit_to_reason("reason_subreddit_whitelist");
}

void MainTab::rm_from_reason_subreddit_wl(){
    this->rm_subreddit_from_reason("reason_subreddit_whitelist");
}


void MainTab::add_to_reason_subreddit_bl(){
    this->add_subreddit_to_reason("reason_subreddit_blacklist");
}

void MainTab::rm_from_reason_subreddit_bl(){
    this->rm_subreddit_from_reason("reason_subreddit_blacklist");
}


