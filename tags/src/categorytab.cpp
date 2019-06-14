#include "categorytab.hpp"

#include <map>

#include <QDialogButtonBox>

#include <compsky/mysql/query.hpp>

#include "clbtn.hpp"
#include "tagdialog.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;

extern std::map<QString, uint64_t> tag_name2id;
extern QStringList tagslist;


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
    
    while (compsky::mysql::assign_next_row(RES2, &ROW2, &id, &name, &r, &g, &b, &a)){
        l->addWidget(new SelectColourButton(id, r, g, b, a, name, this));
    }
    }
    
    setLayout(l);
}

uint64_t ClTagsTab::create_tag(QString& qs,  const char* s){
    constexpr static const compsky::asciify::flag::Escape f;
    compsky::mysql::exec("INSERT INTO tag (name, r,g,b,a) VALUES (\"",  f,  '"',  s,  "\",0,0,0,0)");
    compsky::mysql::query_buffer(&RES1,  "SELECT LAST_INSERT_ID() as ''");
    uint64_t id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &id));
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
