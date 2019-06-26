/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "io_tab.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>


IOTab::IOTab(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* l = new QVBoxLayout(this);
    
    l->addWidget(new QLabel("Placeholder Tab", this));
    l->addWidget(new QLabel("To export tables, navigate to the output directory and use rscraper-export", this));
    l->addWidget(new QLabel("To import tables, navigate to the directory and use rscraper-import", this));
    l->addWidget(new QLineEdit("Check https://notcompsky.github.io/RScraper/Tagger/ for links to data dumps that you can import", this));
    
    this->setLayout(l);
}
