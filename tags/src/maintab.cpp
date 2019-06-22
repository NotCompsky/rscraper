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


int MainTab::add(const char* title,  const char* typ,  const char* tblname,  MainTabMemberFnct f_add,  MainTabMemberFnct f_rm,  QGridLayout* l,  int row){
    l->addWidget(new WlBlLabel(title, typ, tblname),  row,  0);
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
        "subreddit_contents_wl",
        &MainTab::add_to_subreddit_contents_wl,
        &MainTab::rm_from_subreddit_contents_wl,
        l,
        row
    );
    row = add(
        "Blacklist",
        "subreddit",
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
        "user_contents_wl",
        &MainTab::add_to_user_contents_wl,
        &MainTab::rm_from_user_contents_wl,
        l,
        row
    );
    row = add(
        "Blacklist",
        "user",
        "user_contents_bl",
        &MainTab::add_to_user_contents_bl,
        &MainTab::rm_from_user_contents_bl,
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
    if (dialog->exec() != QDialog::Accepted)
        return;
    const QString qstr = dialog->name_edit->text();
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT IGNORE INTO ", tblname, " SELECT id FROM subreddit WHERE name=\"", qstr, "\"");
}

void MainTab::rm_subreddit_from(const char* tblname){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "");
    dialog->name_edit->setCompleter(subreddit_name_completer);
    if (dialog->exec() != QDialog::Accepted)
        return;
    const QString qstr = dialog->name_edit->text();
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("DELETE a FROM ", tblname, " a, subreddit b WHERE a.id=b.id AND b.name=\"", qstr, "\"");
}

void MainTab::add_user_to(const char* tblname){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "");
    if (user_name_completer == nullptr)
        populate_user_name_completer();
    dialog->name_edit->setCompleter(user_name_completer);
    if (dialog->exec() != QDialog::Accepted)
        return;
    const QString qstr = dialog->name_edit->text();
    if (qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT IGNORE INTO ", tblname, " SELECT id FROM user WHERE name=\"", qstr, "\"");
}

void MainTab::rm_user_from(const char* tblname){
    bool ok;
    NameDialog* dialog = new NameDialog(tblname, "");
    if (user_name_completer == nullptr)
        populate_user_name_completer();
    dialog->name_edit->setCompleter(user_name_completer);
    if (dialog->exec() != QDialog::Accepted)
        return;
    const QString qstr = dialog->name_edit->text();
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


