/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "tag_stats.hpp"

#include <QLayout>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QHorizontalBarSeries>
#include <QtCharts/QValueAxis>

#include <compsky/mysql/query.hpp>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern MYSQL_RES* RES2;
extern MYSQL_ROW ROW2;


TagStats::TagStats(uint64_t tag_id,  QWidget* parent)
:
    QDialog(parent), tag_id(tag_id), is_initialised(false)
{}

void TagStats::show_chart(){
    if (!this->is_initialised)
        this->init();
    this->show();
}

void TagStats::init(){
    using namespace QtCharts;
    
    constexpr static const compsky::asciify::flag::Escape f_esc;
    compsky::mysql::query(
        &RES1,
        "SELECT s.name, COUNT" // Only difference
        "(u2scc.count) as c FROM user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit s WHERE u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t.id AND t.id=",
        this->tag_id,
        " AND s2t.subreddit_id=s.id GROUP BY s.name ORDER BY s.id"
    );
    compsky::mysql::query(
        &RES2,
        "SELECT SUM" // Only difference
        "(u2scc.count) as c FROM user2subreddit_cmnt_count u2scc, subreddit2tag s2t, tag t, subreddit s WHERE u2scc.subreddit_id=s2t.subreddit_id AND s2t.tag_id=t.id AND t.id=",
        this->tag_id,
        " AND s2t.subreddit_id=s.id GROUP BY s.name ORDER BY s.id"
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
    
    this->is_initialised = true;
    
    QLayout* l;
    l->addWidget(chart_view);
    this->setLayout(l);
}
