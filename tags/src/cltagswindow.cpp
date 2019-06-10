#include "cltagswindow.h"

#include <QColorDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QTimer>

#include <compsky/mysql/query.hpp>

#define DIGITS_IN_UINT64 19


namespace compsky::asciify {
    char* BUF = (char*)malloc(4096);
}

MYSQL_RES* RES1;
MYSQL_ROW ROW1;

MYSQL_RES* RES2;
MYSQL_ROW ROW2;


QString DISPLAY_TAGS_RES = "";

std::map<QString, uint64_t> tag_name2id;
QStringList tagslist;
QCompleter* tagcompleter;


TagDialog::TagDialog(QString title,  QString str,  QWidget* parent) : QDialog(parent){
    // If the functions are implemented in the header file you have to declare the definitions of the functions with inline to prevent having multiple definitions of the functions.
    this->btn_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(this->btn_box, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this->btn_box, SIGNAL(rejected()), this, SLOT(reject()));
    l = new QVBoxLayout;
    l->addWidget(this->btn_box);
    this->name_edit = new QLineEdit(str);
    tagcompleter = new QCompleter(tagslist);
    this->name_edit->setCompleter(tagcompleter);
    l->addWidget(this->name_edit);
    this->setLayout(l);
    this->setWindowTitle(title);
    QTimer::singleShot(0, this->name_edit, SLOT(setFocus())); // Set focus after TagDialog instance is visible
}

TagDialog::~TagDialog(){
    delete this->name_edit;
    delete this->l;
    delete this->btn_box;
}


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
    
    auto f = compsky::asciify::flag::guarantee::between_zero_and_one_inclusive;
    
    compsky::mysql::exec("UPDATE tag SET r=", f, r, 3, ",g=", f, g, 3, ",b=", f, b, 3, ",a=", f, a, 3, " WHERE id=", this->tag_id);
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
    compsky::mysql::query(&RES1,  "SELECT r.name FROM subreddit r JOIN (SELECT subreddit_id FROM subreddit2tag WHERE tag_id=",  this->tag_id,  ") A ON A.subreddit_id = r.id");
    
    char* name;
    while (compsky::mysql::assign_next_result(RES1, &ROW1, &name)){
        DISPLAY_TAGS_RES += name;
        DISPLAY_TAGS_RES += '\n';
    }
    
    QMessageBox::information(this, tr("Tagged Subreddits"), DISPLAY_TAGS_RES, QMessageBox::Cancel);
    
    DISPLAY_TAGS_RES = "";
}


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

ClTagsTab::ClTagsTab(const uint64_t cat_id, QWidget* parent) : cat_id(cat_id), QWidget(parent){
    QVBoxLayout* l = new QVBoxLayout;
    
    QPushButton* add_tag_btn = new QPushButton("+", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_tag()));
    l->addWidget(add_tag_btn);
    
    compsky::mysql::query(&RES2,  "SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id=",  cat_id,  ") ORDER BY name");
    
    {
    uint64_t id;
    char* name;
    unsigned char r, g, b, a;
    
    while (compsky::mysql::assign_next_result(RES2, &ROW2, &id, &name, &r, &g, &b, &a)){
        l->addWidget(new SelectColourButton(id, r, g, b, a, name, this));
    }
    }
    
    setLayout(l);
}

uint64_t ClTagsTab::create_tag(QString& qs,  const char* s){
    constexpr static auto f = compsky::asciify::flag::escape;
    compsky::mysql::exec("INSERT INTO tag (name, r,g,b,a) VALUES (\"",  f,  '"',  s,  "\",0,0,0,0)");
    compsky::mysql::query_buffer(&RES1,  "SELECT LAST_INSERT_ID() as ''");
    uint64_t id = 0;
    while(compsky::mysql::assign_next_result(RES1, &ROW1, &id));
    tag_name2id[qs] = id;
    return id;
}

void ClTagsTab::add_tag(){
    bool ok;
    TagDialog* tagdialog = new TagDialog("Tag", "");
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    QString tagstr = tagdialog->name_edit->text();
    if (tagstr.isEmpty())
        return;
    
    const QByteArray ba = tagstr.toLocal8Bit();
    const char* tag_str = ba.data();
    
    uint64_t tag_id;
    if (!tagslist.contains(tagstr))
        tag_id = this->create_tag(tagstr, tag_str);
    else
        tag_id = tag_name2id[tagstr];
    
    compsky::mysql::exec("INSERT INTO tag2category (category_id, tag_id) VALUES (",  this->cat_id,  ',',  tag_id,  ')');
    l->addWidget(new SelectColourButton(tag_id, 0, 0, 0, 0, tag_str, this));
}


