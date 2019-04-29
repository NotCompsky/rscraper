#include "cltagswindow.h"
#include "utils.h" // for itoa_nonstandard
#include <QColorDialog>
#include <QVBoxLayout>
#include <QDebug>
#include <QMessageBox>

/* MySQL */
#include <cppconn/driver.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Driver* SQL_DRIVER = get_driver_instance();
sql::Connection* SQL_CON;
sql::Statement* SQL_STMT;
sql::ResultSet* SQL_RES;


#define DIGITS_IN_UINT64 19
constexpr const char* STMT_GETCATTAGS_STR = "SELECT id, name, r, g, b, a FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id = ";
char STMT_GETCATTAGS[strlen(STMT_GETCATTAGS_STR) + DIGITS_IN_UINT64 + strlen(" ORDER BY name") + 1] = "SELECT id, name, r, g, b, a FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id = ";

constexpr const char* STMT_SETCL_STR = "UPDATE tag SET ";
char STMT_SETCL[strlen("UPDATE tag SET r=0.123,g=0.123,b=0.123,a=0.123 WHERE id=01234567890123456789") + 1] = "UPDATE tag SET r=";


#define SUBREDDIT_NAME_MAX 128
constexpr const char* SQL__SELECT_SUBS_W_TAG_PRE = "SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id = ";
constexpr const char* SQL__SELECT_SUBS_W_TAG_POST = ") A ON A.subreddit_id = r.id";
char SQL__SELECT_SUBS_W_TAG[strlen(SQL__SELECT_SUBS_W_TAG_PRE) + 20 + strlen(SQL__SELECT_SUBS_W_TAG_POST) + 1] = "SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id = ";

/*char* DISPLAY_TAGS_RES = (char*)malloc(4096);
size_t DISPLAY_TAGS_RES_SIZE = 4096;*/
QString DISPLAY_TAGS_RES = "";


void format_stmt_setcl(uint64_t id, int ir, int ig, int ib, int ia){
    int i = strlen(STMT_SETCL_STR);
    
    char labels[4] = {'r', 'g', 'b', 'a'};
    double rgba[4] = {(double)ir/255.0, (double)ig/255.0, (double)ib/255.0, (double)ia/255.0};
    
    for (auto k = 0;  k < 4;  ++k){
        STMT_SETCL[i++] = labels[k];
        STMT_SETCL[i++] = '=';
        if (rgba[k] == 1){
            STMT_SETCL[i++] = '1';
            // All remaining decimals are zero
            STMT_SETCL[i++] = '.';
            STMT_SETCL[i++] = '0';
            STMT_SETCL[i++] = ',';
            continue;
        }
        STMT_SETCL[i++] = '0';
        STMT_SETCL[i++] = '.';
        for (auto j = 0;  j < 4;  ++j){
            rgba[k] *= 10;
            STMT_SETCL[i++] = '0' + (char)rgba[k];
            rgba[k] -= (char)rgba[k];
        }
        STMT_SETCL[i++] = ',';
    }
    STMT_SETCL[i - 1] = ' '; // Overwrite trailing comma
    
    memcpy(STMT_SETCL + i,  "WHERE id = ",  strlen("WHERE id = "));
    i += strlen("WHERE id = ");
    
    i += itoa_nonstandard(id,  STMT_SETCL + i);
    
    STMT_SETCL[i] = 0;
}


SelectColourButton::SelectColourButton(const uint64_t id, const double r, const double g, const double b, const double a, const char* name, QWidget* parent){
    this->tag_id = id;
    this->setText(name);
    this->setAutoFillBackground(true);
    this->setFlat(true);
    this->colour = QColor(255*r, 255*g, 255*b, 255*a);
    
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, this->colour);
    this->setPalette(pal);
    this->update();
}

void SelectColourButton::set_colour(){
    this->colour = QColorDialog::getColor(this->colour, parentWidget());
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, this->colour);
    this->setPalette(pal);
    this->update();
    
    int r, g, b, a;
    this->colour.getRgb(&r, &g, &b, &a);
    format_stmt_setcl(this->tag_id, r, g, b, a);
    qDebug() << STMT_SETCL;
    SQL_STMT->execute(STMT_SETCL);
}

void SelectColourButton::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            this->set_colour();
            return;
        case Qt::RightButton:
            this->display_subs_w_tag();
            return;
    }
}

/*
void resize_display_tags_res(size_t n){
    DISPLAY_TAGS_RES_SIZE = 2*n;
    DISPLAY_TAGS_RES = (char*)realloc(DISPLAY_TAGS_RES, DISPLAY_TAGS_RES_SIZE); // TODO: Check for nullptr
}
*/

void SelectColourButton::display_subs_w_tag(){
    int i = strlen(SQL__SELECT_SUBS_W_TAG_PRE);
    i += itoa_nonstandard(this->tag_id,  SQL__SELECT_SUBS_W_TAG + i);
    memcpy(SQL__SELECT_SUBS_W_TAG + i,  SQL__SELECT_SUBS_W_TAG_POST,  strlen(SQL__SELECT_SUBS_W_TAG_POST));
    i += strlen(SQL__SELECT_SUBS_W_TAG_POST);
    SQL__SELECT_SUBS_W_TAG[i] = 0;
    
    qDebug() << SQL__SELECT_SUBS_W_TAG;
    
    SQL_RES = SQL_STMT->executeQuery(SQL__SELECT_SUBS_W_TAG);
    
    //size_t j = 0;
    
    while (SQL_RES->next()){
        const std::string ss = SQL_RES->getString(1);
        DISPLAY_TAGS_RES += QString::fromStdString(ss);
        DISPLAY_TAGS_RES += '\n';
        /*const char* s        = ss.c_str();
        memcpy(DISPLAY_TAGS_RES + j,  s,  strlen(s));
        j += strlen(s);
        DISPLAY_TAGS_RES[j++] = '\n';
        if (j > DISPLAY_TAGS_RES_SIZE + SUBREDDIT_NAME_MAX)
            resize_display_tags_res(j);*/
    }
    
    /*DISPLAY_TAGS_RES[i] = 0;
    QString qstr(DISPLAY_TAGS_RES);*/
    
    QMessageBox::information(this, tr("Tagged Subreddits"), DISPLAY_TAGS_RES, QMessageBox::Cancel);
    
    DISPLAY_TAGS_RES = "";
}


ClTagsDialog::ClTagsDialog(QWidget* parent){
    QTabWidget* tabWidget = new QTabWidget;
    
    
    SQL_CON = SQL_DRIVER->connect("unix:///var/run/mysqld/mysqld.sock", "rscraper++", "***REMOVED***");
    SQL_CON->setSchema("rscraper");
    SQL_STMT = SQL_CON->createStatement();
    
    sql::ResultSet* sql_res = SQL_STMT->executeQuery("SELECT id, name FROM category");
    while (sql_res->next()){
        const uint64_t id = sql_res->getUInt64(1);
        const std::string sname = sql_res->getString(2);
        const char*        name = sname.c_str();
        
        tabWidget->addTab(new ClTagsTab(id), tr(name));
    }
    
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(tabWidget);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
    
    setWindowTitle(tr("rscraper++ tag colour picker"));
}

ClTagsTab::ClTagsTab(const uint64_t id, QWidget* parent) : QWidget(parent){
    QVBoxLayout* mainLayout = new QVBoxLayout;
    
    int i = strlen(STMT_GETCATTAGS_STR);
    i += itoa_nonstandard(id,  STMT_GETCATTAGS + i);
    STMT_GETCATTAGS[i++] = ')';
    
    memcpy(STMT_GETCATTAGS + i,  " ORDER BY NAME",  strlen(" ORDER BY NAME"));
    i += strlen(" ORDER BY NAME");
    
    STMT_GETCATTAGS[i] = 0;
    
    SQL_RES = SQL_STMT->executeQuery(STMT_GETCATTAGS);
    while (SQL_RES->next()){
        const uint64_t id = SQL_RES->getUInt64(1);
        const std::string sname = SQL_RES->getString(2);
        const char*        name = sname.c_str();
        const double r = SQL_RES->getDouble(3); // Should be getFloat, but no such function
        const double g = SQL_RES->getDouble(4);
        const double b = SQL_RES->getDouble(5);
        const double a = SQL_RES->getDouble(6);
        
        mainLayout->addWidget(new SelectColourButton(id, r, g, b, a, name, this));
    }
    
    setLayout(mainLayout);
}
