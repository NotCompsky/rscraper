#include "cltagswindow.h"

#include <QColorDialog>
#include <QVBoxLayout>
#include <QMessageBox>

#include "mymysql.hpp" // for mymysql::*

namespace res1 {
    #include "mymysql_results.hpp"
}

namespace res2 {
    #include "mymysql_results.hpp"
}

#define DIGITS_IN_UINT64 19


constexpr const int BUF_SZ_INIT = 4096;
char* BUF = (char*)malloc(BUF_SZ_INIT);
int BUF_INDX = 0;
int BUF_SZ = BUF_SZ_INIT;


QString DISPLAY_TAGS_RES = "";


SelectColourButton::SelectColourButton(const uint64_t id,  const unsigned char r,  const unsigned char g,  const unsigned char b,  const unsigned char a,  const char* name,  QWidget* parent){
    this->tag_id = id;
    this->setText(name);
    this->setAutoFillBackground(true);
    this->setFlat(true);
    this->colour = QColor(r, g, b, a);
    
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
    
    int ir, ig, ib, ia;
    this->colour.getRgb(&ir, &ig, &ib, &ia);
    
    const double r = ir/255.0;
    const double g = ig/255.0;
    const double b = ib/255.0;
    const double a = ia/255.0;
    
    mymysql::exec("UPDATE tag SET r=", r, 3, ",g=", g, 3, ",b=", b, 3, ",a=", a, 3, " WHERE id=", this->tag_id);
    BUF_INDX = 0;
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
    res1::query("SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id=",  this->tag_id,  ") A ON A.subreddit_id = r.id");
    BUF_INDX = 0;
    
    char* name;
    while (res1::assign_next_result(&name)){
        DISPLAY_TAGS_RES += name;
        DISPLAY_TAGS_RES += '\n';
    }
    
    res1::free_result();
    
    QMessageBox::information(this, tr("Tagged Subreddits"), DISPLAY_TAGS_RES, QMessageBox::Cancel);
    
    DISPLAY_TAGS_RES = "";
}


ClTagsDialog::ClTagsDialog(const char* mysql_cfg,  QWidget* parent){
    mymysql::init(mysql_cfg);
    
    QTabWidget* tabWidget = new QTabWidget;
    
    res1::query("SELECT id, name FROM category");
    BUF_INDX = 0;
    
    {
    uint64_t id;
    char* name;
    while (res1::assign_next_result(&id, &name)){
        tabWidget->addTab(new ClTagsTab(id), tr(name));
    }
    res1::free_result();
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

ClTagsDialog::~ClTagsDialog(){
    mymysql::exit();
}

ClTagsTab::ClTagsTab(const uint64_t id, QWidget* parent) : QWidget(parent){
    QVBoxLayout* mainLayout = new QVBoxLayout;
    
    res2::query("SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id=",  id,  ") ORDER BY name");
    BUF_INDX = 0;
    
    {
    uint64_t id;
    char* name;
    unsigned char r, g, b, a;
    
    while (res2::assign_next_result(&id, &name, &r, &g, &b, &a)){
        mainLayout->addWidget(new SelectColourButton(id, r, g, b, a, name, this));
    }
    res2::free_result();
    }
    
    setLayout(mainLayout);
}
