/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "main_tab.hpp"

#include "categorytab.hpp"
#include "name_dialog.hpp"

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <QLabel>
#include <QPushButton>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

namespace _f {
    constexpr static const compsky::asciify::flag::Escape esc;
}


MainTab::MainTab(QTabWidget* tab_widget,  QWidget* parent) : QWidget(parent), tab_widget(tab_widget) {
    QGridLayout* l = new QGridLayout;
    
    int row = 0;
    
    
    QPushButton* add_tag_btn = new QPushButton("+Category", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_category()));
    l->addWidget(add_tag_btn, row++, 0);
    
    
    // TODO: Move scraper config rows to a different section
    {
    l->addWidget(new QLabel("Doughnut"), row, 0);
    this->cat_doughnut = new CatDoughnut(this);
    QPushButton* bake_cat_doughnut_btn = new QPushButton("Bake", this);
    connect(bake_cat_doughnut_btn, &QPushButton::clicked, this->cat_doughnut, &CatDoughnut::init);
    l->addWidget(bake_cat_doughnut_btn, row, 1);
    QPushButton* show_cat_doughnut_btn = new QPushButton("Eat", this);
    connect(show_cat_doughnut_btn, &QPushButton::clicked, this->cat_doughnut, &CatDoughnut::show_chart);
    l->addWidget(show_cat_doughnut_btn, row, 2);
    }
    
    
    setLayout(l);
}

void MainTab::add_category(){
    bool ok;
    NameDialog* catdialog = new NameDialog("New Category", "Avoid using '&'");
    const int rc = catdialog->exec();
    const QString cat_qstr = catdialog->name_edit->text();
    delete catdialog;
    if (rc != QDialog::Accepted  ||  cat_qstr.isEmpty())
        return;
    
    compsky::mysql::exec("INSERT INTO category (name) VALUES (\"", _f::esc, '"', cat_qstr, "\")");
    
    compsky::mysql::query(&RES1, "SELECT id FROM category WHERE name=\"", _f::esc, '"', cat_qstr, "\"");
    
    uint64_t cat_id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &cat_id));
    
    this->tab_widget->addTab(new ClTagsTab(cat_id, this->tab_widget), cat_qstr);
}
