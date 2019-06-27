/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include "categorytab.hpp"

#include <map>

#include <QCompleter>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>

#include <compsky/mysql/query.hpp>

#include "add_sub2tag_btn.hpp"
#include "btn_with_id.hpp"
#include "rm_tag_btn.hpp"
#include "clbtn.hpp"
#include "mv_tag_btn.hpp"
#include "name_dialog.hpp"
#include "tagnamelabel.hpp"
#include "tag_stats.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;
extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;

extern std::map<QString, uint64_t> tag_name2id;
extern QStringList tagslist;


ClTagsTab::ClTagsTab(const uint64_t cat_id,  QTabWidget* tab_widget,  QWidget* parent) : cat_id(cat_id), QWidget(parent), row(0), tab_widget(tab_widget){
    this->l = new QGridLayout;
    
    QPushButton* add_tag_btn = new QPushButton("+Tag", this);
    connect(add_tag_btn, SIGNAL(clicked()), this, SLOT(add_tag()));
    this->l->addWidget(add_tag_btn, 0, 0);
    
    compsky::mysql::query(&RES2,  "SELECT id, name, FLOOR(255*r), FLOOR(255*g), FLOOR(255*b), FLOOR(255*a) FROM tag WHERE id IN (SELECT tag_id FROM tag2category WHERE category_id=",  cat_id,  ") ORDER BY name");
    
    {
    uint64_t id;
    char* name;
    unsigned char r, g, b, a;
    
    while (compsky::mysql::assign_next_row(RES2, &ROW2, &id, &name, &r, &g, &b, &a)){
        ++this->row;
        this->l->addWidget(new TagNameLabel(id, name, this),                this->row,  0);
        this->l->addWidget(new SelectColourButton(id, r, g, b, a, this),    this->row,  1);
        
        BtnWithID* tag_stats_btn = new BtnWithID("Stats", id, this);
        connect(tag_stats_btn, &BtnWithID::left_clicked, this, &ClTagsTab::display_tag_stats);
        this->l->addWidget(tag_stats_btn, this->row, 2);
        
        this->l->addWidget(new AddSub2TagBtn(id, false, this),  this->row,  3);
        this->l->addWidget(new AddSub2TagBtn(id, true,  this),  this->row,  4);
        this->l->addWidget(new MvTagBtn(id, this),  this->row,  5);
        this->l->addWidget(new RmTagBtn(id, this),  this->row,  6);
    }
    }
    
    QPushButton* rm_self_btn = new QPushButton("Delete Category");
    connect(rm_self_btn, SIGNAL(clicked()), this, SLOT(rm_self()));
    this->l->addWidget(rm_self_btn, ++this->row, 0);
    
    setLayout(this->l);
}

uint64_t ClTagsTab::create_tag(QString& qs){
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
    ++this->row;
    tagslist << tagstr;
    this->l->addWidget(new TagNameLabel(tag_id, tagstr, this),              this->row,  0);
    this->l->addWidget(new SelectColourButton(tag_id,  0, 0, 0, 0,  this),  this->row,  1);
    
    BtnWithID* tag_stats_btn = new BtnWithID("Stats", tag_id, this);
    connect(tag_stats_btn, &BtnWithID::left_clicked, this, &ClTagsTab::display_tag_stats);
    this->l->addWidget(tag_stats_btn, this->row, 2);
    
    this->l->addWidget(new AddSub2TagBtn(tag_id, false, this),  this->row,  3);
    this->l->addWidget(new AddSub2TagBtn(tag_id, true,  this),  this->row,  4);
    this->l->addWidget(new MvTagBtn(tag_id, this),  this->row,  5);
    this->l->addWidget(new RmTagBtn(tag_id, this),  this->row,  6);
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
    this->tab_widget->removeTab(this->tab_widget->indexOf(this));
    delete this;
}

void ClTagsTab::display_tag_stats(const int tag_id){
    using namespace QtCharts;
    
    constexpr static const compsky::asciify::flag::Escape f_esc;
    compsky::mysql::query(
        &RES1,
        "SELECT s.name, COUNT" // Only difference
        "(u2scc.count) as c FROM user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit s WHERE u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t.id AND t.id=",
        tag_id,
        " AND s2t.subreddit_id=s.id GROUP BY s.name ORDER BY s.name"
    );
    compsky::mysql::query(
        &RES2,
        "SELECT SUM" // Only difference
        "(u2scc.count) as c FROM user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit s WHERE u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t.id AND t.id=",
        tag_id,
        " AND s2t.subreddit_id=s.id GROUP BY s.name ORDER BY s.name"
    );
    
    char* name;
    uint64_t count;
    uint64_t sum;
    QHorizontalBarSeries* series = new QHorizontalBarSeries();
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &name, &count)  &&  compsky::mysql::assign_next_row(RES2, &ROW1, &sum)){
        QBarSet* set = new QBarSet(name);
        set->append(count);
        set->append(sum);
        series->append(set);
    }
    
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Sums per tagged subreddit");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    
    QStringList categories;
    categories << "Users" << "Comments";
    QBarCategoryAxis* axis_y = new QBarCategoryAxis();
    axis_y->append(categories);
    chart->addAxis(axis_y, Qt::AlignLeft);
    series->attachAxis(axis_y);
    QValueAxis* axis_x = new QValueAxis();
    chart->addAxis(axis_x, Qt::AlignBottom);
    series->attachAxis(axis_x);
    axis_x->applyNiceNumbers();
    
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    QChartView* chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    
    QDialog* dialog = new QDialog(this);
    
    QVBoxLayout* l = new QVBoxLayout();
    l->addWidget(chart_view);
    dialog->setLayout(l);
    
    dialog->show();
}
