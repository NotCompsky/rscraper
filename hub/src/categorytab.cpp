/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "categorytab.hpp"

#include "add_sub2tag_btn.hpp"
#include "btn_with_id.hpp"
#include "cat_pie.hpp"
#include "clbtn.hpp"
#include "name_dialog.hpp"
#include "tag_pie.hpp"
#include "sh_tag_btn.hpp"
#include "unlink_tag_btn.hpp"
#include "rm_tag_btn.hpp"
#include "tagnamelabel.hpp"

#include <compsky/mysql/query.hpp>

#include <map>

#include <QCompleter>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QValueAxis>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;

extern std::map<QString, uint64_t> tag_name2id;
extern QStringList tagslist;
extern QStringList category_names;


ClTagsTab::ClTagsTab(const uint64_t cat_id,  QTabWidget* tab_widget,  QWidget* parent) : cat_id(cat_id), QWidget(parent), row(0), tab_widget(tab_widget){
    this->l = new QGridLayout;
    
    {
    l->addWidget(new QLabel("Pie Chart"), ++this->row, 0);
    this->cat_pie = new CatPie(this->cat_id, this);
    QPushButton* bake_cat_pie_btn = new QPushButton("Bake", this);
    connect(bake_cat_pie_btn, &QPushButton::clicked, this->cat_pie, &CatPie::init);
    l->addWidget(bake_cat_pie_btn, this->row, 1);
    QPushButton* show_cat_pie_btn = new QPushButton("Eat", this);
    connect(show_cat_pie_btn, &QPushButton::clicked, this->cat_pie, &CatPie::show_chart);
    l->addWidget(show_cat_pie_btn, this->row, 2);
    ++this->row;
    }
    
    QPushButton* add_tag_btn = new QPushButton("+Tag", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_tag()));
    this->l->addWidget(add_tag_btn, 0, 0);
    
    compsky::mysql::query(&RES2,  "SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id=",  cat_id,  ") ORDER BY name");
    
    {
    uint64_t id;
    char* name;
    uint8_t r, g, b, a;
    
    while (compsky::mysql::assign_next_row(RES2, &ROW2, &id, &name, &r, &g, &b, &a))
        add_tag_row(id, name, QColor(r, g, b, a));
    }
    
    
    QPushButton* rm_self_btn = new QPushButton("Delete Category");
    connect(rm_self_btn, SIGNAL(clicked()), this, SLOT(rm_self()));
    this->l->addWidget(rm_self_btn, ++this->row, 0);
    
    setLayout(this->l);
}

QScrollArea* ClTagsTab::tab_named(const QString& qs){
    QString qstr = qs;
    qstr.detach(); // i.e. makes deep copy
    qstr.remove('&');
    // Remove ampersands that are automatically added by Qt to control the shortcuts (Alt+char)
    // The shortcutting cannot be disabled unless Qt is compiled with QT_NO_SHORTCUT
    // Additionally, even when shortcutting is disabled, single ampersands do not appear, and double ampersands are converted to single ampersands.
    // see https://stackoverflow.com/questions/52266876/how-do-i-disable-special-handling-of-on-qt-button-labels
    for (auto i = 0;  i < this->tab_widget->count();  ++i)
        if (this->tab_widget->tabText(i).remove('&') == qstr.remove('&'))
            return static_cast<QScrollArea*>(this->tab_widget->widget(i));
    return nullptr;
}

uint64_t ClTagsTab::create_tag(const QString& qs){
    constexpr static const compsky::asciify::flag::Escape f;
    compsky::mysql::exec("INSERT IGNORE INTO tag (name, r,g,b,a) VALUES (\"",  f,  '"',  qs,  "\",0,0,0,0)");
    compsky::mysql::query_buffer(&RES1,  "SELECT LAST_INSERT_ID() as ''");
    uint64_t id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &id));
    tag_name2id[qs] = id;
    return id;
}

void ClTagsTab::add_tag(){
    bool ok;
    NameDialog* tagdialog = new NameDialog("Tag", "");
    QCompleter* tagcompleter = new QCompleter(tagslist);
    tagdialog->name_edit->setCompleter(tagcompleter);
    if (tagdialog->exec() != QDialog::Accepted)
        return;
    QString tagstr = tagdialog->name_edit->text();
    delete tagdialog;
    if (tagstr.isEmpty())
        return;
    
    const uint64_t tag_id  =  (tagslist.contains(tagstr))  ?  tag_name2id[tagstr]  :  this->create_tag(tagstr);
    
    compsky::mysql::exec("INSERT IGNORE INTO tag2category (category_id, tag_id) VALUES (",  this->cat_id,  ',',  tag_id,  ')');
    tagslist << tagstr;
    
    this->add_tag_row(tag_id,  tagstr,  QColor(0, 0, 0, 0));
}

void ClTagsTab::add_tag_row(const uint64_t tag_id,  QString tagstr,  const QColor& cl){
    ++this->row;
    
    this->l->addWidget(new TagNameLabel(tag_id, tagstr, this),              this->row,  0);
    this->l->addWidget(new SelectColourButton(tag_id,  cl,  this),  this->row,  1);
    
    BtnWithID* tag_stats_btn = new BtnWithID("Stats", tag_id, this);
    connect(tag_stats_btn, &BtnWithID::left_clicked, this, &ClTagsTab::display_tag_stats);
    this->l->addWidget(tag_stats_btn, this->row, 2);
    
    this->l->addWidget(new AddSub2TagBtn(tag_id, false, this),  this->row,  3);
    this->l->addWidget(new AddSub2TagBtn(tag_id, true,  this),  this->row,  4);
    this->l->addWidget(new ShTagBtn(tag_id,     this),  this->row,  5);
    this->l->addWidget(new UnlinkTagBtn(tag_id, this),  this->row,  6);
    this->l->addWidget(new RmTagBtn(tag_id,     this),  this->row,  7);
}

void ClTagsTab::rm_self(){
    // For safety reasons, only empty categories will be deleted
    compsky::mysql::query(&RES1, "SELECT t2c.tag_id, t.name FROM tag2category t2c, tag t WHERE t.id=t2c.tag_id AND t2c.category_id=", this->cat_id);
    uint64_t tag_id = 0;
    char* name;
    QString s = "Refusing to delete non-empty category.\nTags with this category are:";
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &tag_id, &name)){
        s += "\n";
        s += name;
    }
    if (tag_id != 0){
        QMessageBox::information(this, tr("Error"), s);
        return;
    }
    compsky::mysql::exec("DELETE FROM category WHERE id=", this->cat_id);
    
    category_names.removeAt(category_names.indexOf(this->tab_widget->tabText(this->tab_widget->indexOf(this))));
    
    this->tab_widget->removeTab(this->tab_widget->indexOf(this));
    delete this;
}

void ClTagsTab::display_tag_stats(const int tag_id){
    TagPie* tag_pie = new TagPie(tag_id, this);
    tag_pie->exec();
}
