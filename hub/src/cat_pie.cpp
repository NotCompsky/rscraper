/*
 * Based on example given in Qt documentation: https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-example.html
 */

#include "cat_pie.hpp"

#include <compsky/mysql/query.hpp>

#include <QChartView>
#include <QMessageBox>
#include <QPieLegendMarker>
#include <QPieSeries>
#include <QPieSlice>
#include <QPushButton>
#include <QVBoxLayout>


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;


CatPie::CatPie(const uint64_t category_id,  QWidget* parent) : QDialog(parent), chart(nullptr), cat_id(category_id), is_initialised(false) {}

void CatPie::show_chart(){
    if (!is_initialised){
        QMessageBox::information(this, "Uninitialised", "Bake chart first (allow ~1 minute)");
        return;
    }
    this->show();
}

void CatPie::init(){
    using namespace QtCharts;
    
    // TODO: Use worker thread for this
    compsky::mysql::query(&RES1,  "SELECT t.name, COUNT(u2scc.count), FLOOR(255*r) as r, FLOOR(255*g) as g, FLOOR(255*b) as b, FLOOR(255*a) as a FROM tag2category t2c, tag t, subreddit2tag s2t, user2subreddit_cmnt_count u2scc WHERE t2c.category_id=", this->cat_id, " AND t.id=t2c.tag_id AND t.id=s2t.tag_id AND s2t.subreddit_id=u2scc.subreddit_id GROUP BY t.name, r, g, b, a");
    uint64_t tag_count;
    char* tag_name;
    uint8_t r, g, b, a;
    QPieSeries* series = new QPieSeries();
    this->chart = new QChart();
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &tag_name, &tag_count, &r, &g, &b, &a)){
        QPieSlice* slice = series->append(QString(tag_name), tag_count);
        slice->setColor(QColor(r, g, b, a));
    }
    series->setLabelsVisible();
    chart->addSeries(series);
    for (QLegendMarker* marker : this->chart->legend()->markers(series)){
        // This loop is adapted from the Breakdown Chart example in Qt5 documentation
        QPieLegendMarker* pie_marker = qobject_cast<QPieLegendMarker*>(marker);
        pie_marker->setLabel(
            QString("%1 %2%")
                .arg(pie_marker->slice()->label())
                .arg(pie_marker->slice()->percentage() * 100, 0, 'f', 2)
        );
        pie_marker->setFont(QFont("Arial", 8));
    }
    
    this->chart->setAnimationOptions(QChart::AllAnimations);
    this->chart->setTitle("Total comment count per tag");
    this->chart->legend()->setAlignment(Qt::AlignRight);
    
    QVBoxLayout* l = new QVBoxLayout;
    
    QChartView* chart_view = new QChartView(this->chart);
    chart_view->setRubberBand(QChartView::RectangleRubberBand);
    chart_view->setRenderHint(QPainter::Antialiasing);
    l->addWidget(chart_view);
    
    QPushButton* hide_this = new QPushButton("Hide", this);
    connect(hide_this, &QPushButton::clicked, this, &CatPie::hide);
    l->addWidget(hide_this);
    
    this->setLayout(l);
    
    this->is_initialised = true;
}
