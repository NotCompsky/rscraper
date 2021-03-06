/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "mainwindow.hpp"

#include "categorytab.hpp"
#include "reason_tab.hpp"
#include "io_tab.hpp"
#include "main_tab.hpp"
#include "name_dialog.hpp"
#include "scraper_tab.hpp"
#include "view_matched_comments.hpp"

#include <compsky/asciify/flags.hpp>
#include <compsky/mysql/query.hpp>

#include <QColorDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>

#define DIGITS_IN_UINT64 19


char BUF[4096];


namespace _mysql {
	MYSQL* obj;
	
	MYSQL_RES* res1;
	MYSQL_ROW row1;

	MYSQL_RES* res2;
	MYSQL_ROW row2;
	
	char auth[512];
}

std::map<QString, uint64_t> tag_name2id;
QStringList tagslist;
QCompleter* tagcompleter;
QStringList subreddit_names;
QCompleter* subreddit_name_completer;
QStringList category_names;

constexpr static const compsky::asciify::flag::Escape f_esc;


MainWindow::MainWindow(QWidget* parent){
	compsky::mysql::init(_mysql::obj, _mysql::auth, 512, getenv("RSCRAPER_MYSQL_CFG"));
	
	// TODO: Add status bar, to display messages such as "Executing query, might take a while"
	
	this->tab_widget = new QTabWidget(this);
	connect(this->tab_widget, &QTabWidget::tabBarDoubleClicked, this, &MainWindow::rename_category);


	this->tab_widget->addTab(new MainTab(this->tab_widget),             "__MAIN__");
	this->tab_widget->addTab(new ScraperTab(),                          "__SCRAPER__");
	this->tab_widget->addTab(new ViewMatchedComments(this->tab_widget), "__CMNTS__");

	{
	QScrollArea* scroll_area = new QScrollArea(this);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scroll_area->setWidgetResizable(true);
	scroll_area->setWidget(new ReasonTab(this->tab_widget));
	this->tab_widget->addTab(scroll_area, "__REASONS__");
	}

	this->tab_widget->addTab(new IOTab(this->tab_widget),               "__IO__");

	#define N_NONCATEGORY_TABS 5

	
	tag_name2id.clear();
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1, "SELECT id, name FROM tag");
	{
	uint64_t id;
	const char* name;
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &id, &name)){
		tag_name2id[name] = id;
		tagslist << name;
	}
	}
	
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1, "SELECT id, name FROM category");
	
	{
	uint64_t id;
	const char* name;
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &id, &name)){
		this->insert_category(id, name);
		category_names << name;
	}
	}
	
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1, "SELECT name FROM subreddit");
	{
	const char* name;
	while (compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &name)){
		subreddit_names << name;
	}
	}
	subreddit_name_completer = new QCompleter(subreddit_names);
	
	
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &MainWindow::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &MainWindow::reject);
	
	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(this->tab_widget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
	
	setWindowTitle("RScraper Hub");
}

MainWindow::~MainWindow(){
	compsky::mysql::wipe_auth(_mysql::auth, 512);
}

void MainWindow::insert_category(const uint64_t id,  const char* name){
	ClTagsTab* tab = new ClTagsTab(id, this->tab_widget);
	
	QScrollArea* scroll_area = new QScrollArea(this);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	scroll_area->setWidgetResizable(true);
	scroll_area->setWidget(tab);
	
	category_names << name;
	
	this->tab_widget->addTab(scroll_area, name);
}

void MainWindow::rename_category(int indx){
	if (indx < N_NONCATEGORY_TABS)
		// Only category tabs can be renamed
		return;
	
	NameDialog* dialog = new NameDialog("Rename Category", this->tab_widget->tabText(indx));
	const int rc = dialog->exec();
	QString qstr = dialog->name_edit->text();
	delete dialog;
	if (rc != QDialog::Accepted  ||  qstr.isEmpty())
		return;
	this->tab_widget->setTabText(indx, qstr);
	
	const uint64_t cat_id = static_cast<ClTagsTab*>(static_cast<QScrollArea*>(this->tab_widget->widget(indx))->widget())->cat_id;
	
	compsky::mysql::exec(_mysql::obj, BUF, "UPDATE category SET name=\"", f_esc, '"', qstr, "\" WHERE id=", cat_id);
}
