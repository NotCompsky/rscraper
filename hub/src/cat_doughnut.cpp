/*
 * Based on example given in Qt documentation: https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-example.html
 */

#include "cat_doughnut.hpp"

#include <tuple>

#include <QChartView>
#include <QPieSeries>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>

#include <compsky/mysql/query.hpp>

#include "3rdparty/donutbreakdownchart.h"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


CatDoughnut::CatDoughnut(QWidget* parent) : QDialog(parent), is_initialised(false) {}

void CatDoughnut::show_chart(){
    if (!is_initialised)
        this->init();
    this->show();
}

void CatDoughnut::init(){
    compsky::mysql::query_buffer(&RES1,  "SELECT c.id, c.name, t.name, COUNT(u2scc.count) FROM category c, tag2category t2c, tag t, subreddit2tag s2t, user2subreddit_cmnt_count u2scc WHERE t2c.category_id=c.id AND t.id=t2c.tag_id AND t.id=s2t.tag_id AND s2t.subreddit_id=u2scc.subreddit_id GROUP BY c.id, c.name, t.name ORDER BY c.id");
    uint64_t last_cat_id = 0;
    uint64_t cat_id, tag_count;
    char* cat_name;
    char* tag_name;
    QPieSeries* series = nullptr;
    DonutBreakdownChart* chart = new DonutBreakdownChart();
    int r = 255;
    int g = 0;
    int b = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &cat_id, &cat_name, &tag_name, &tag_count)){
        if (cat_id != last_cat_id){
            if (series != nullptr){
                chart->addBreakdownSeries(series, QColor(r, g, b, 255)); // TODO: Mix tag colours together to generate this colour, or allow the assignment of colours to categories themselves
                std::tie(r, g, b) = std::make_tuple(g, b, r);
            }
            series = new QPieSeries();
            series->setName(cat_name);
            last_cat_id = cat_id;
        }
        series->append(QString(tag_name), tag_count);
    }
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTitle("Total comment count per tag category");
    chart->legend()->setAlignment(Qt::AlignRight);
    
    QVBoxLayout* l = new QVBoxLayout;
    
    QChartView* chart_view = new QChartView(chart);
    chart_view->setRenderHint(QPainter::Antialiasing);
    l->addWidget(chart_view);
    
    QPushButton* hide_this = new QPushButton("Hide", this);
    connect(hide_this, &QPushButton::clicked, this, &CatDoughnut::hide);
    l->addWidget(hide_this);
    
    this->setLayout(l);
    
    this->is_initialised = true;
}
