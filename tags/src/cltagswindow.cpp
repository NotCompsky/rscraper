#include "cltagswindow.h"

#include <QColorDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QVBoxLayout>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"

#define DIGITS_IN_UINT64 19


namespace compsky::asciify {
    char* BUF = (char*)malloc(4096);
}

MYSQL_RES* RES1;
MYSQL_ROW ROW1;

MYSQL_RES* RES2;
MYSQL_ROW ROW2;


std::map<QString, uint64_t> tag_name2id;
QStringList tagslist;
QCompleter* tagcompleter;


ClTagsDialog::ClTagsDialog(QWidget* parent){
    compsky::mysql::init(getenv("RSCRAPER_MYSQL_CFG"));
    
    QTabWidget* tabWidget = new QTabWidget;
    
    tag_name2id.clear();
    compsky::mysql::query_buffer(&RES1, "SELECT id, name FROM tag");
    {
    uint64_t id;
    char* name;
    while (compsky::mysql::assign_next_result(RES1, &ROW1, &id, &name)){
        tag_name2id[name] = id;
        tagslist << name;
    }
    }
    
    compsky::mysql::query_buffer(&RES1, "SELECT id, name FROM category");
    
    {
    uint64_t id;
    char* name;
    while (compsky::mysql::assign_next_result(RES1, &ROW1, &id, &name)){
        tabWidget->addTab(new ClTagsTab(id), tr(name));
    }
    }
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    
    setWindowTitle(tr("rscraper tag colour picker"));
}

ClTagsDialog::~ClTagsDialog(){
    compsky::mysql::exit();
}
