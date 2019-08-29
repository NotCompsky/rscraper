/*
 * Based on example given in Qt documentation: https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-example.html
 */

#include "cat_doughnut.hpp"
#include "mysql_declarations.hpp"
#include <compsky/mysql/query.hpp>

#include "3rdparty/donutbreakdownchart.h"
#include "3rdparty/mainslice.h"

#include <tuple>

#include <QChartView>
#include <QMessageBox>
#include <QPieSeries>
#include <QPieSlice>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWindow>


CatDoughnut::CatDoughnut(QWidget* parent) : QDialog(parent), is_initialised(false), chart(nullptr) {}

void CatDoughnut::show_chart(){
	if (!is_initialised){
		QMessageBox::information(this, "Uninitialised", "Bake chart first (allow ~1 minute)");
		return;
	}
	this->show();
}

void CatDoughnut::init(){
	// TODO: Use worker thread for this
	compsky::mysql::query_buffer(_mysql::obj, _mysql::res1,  "SELECT c.id, c.name, t.name, COUNT(u2scc.count), FLOOR(255*r) as r, FLOOR(255*g) as g, FLOOR(255*b) as b, FLOOR(255*a) as a FROM category c, tag2category t2c, tag t, subreddit2tag s2t, user2subreddit_cmnt_count u2scc WHERE t2c.category_id=c.id AND t.id=t2c.tag_id AND t.id=s2t.tag_id AND s2t.subreddit_id=u2scc.subreddit_id GROUP BY c.id, c.name, t.name, r, g, b, a ORDER BY c.id");
	uint64_t last_cat_id = 0;
	uint64_t cat_id, tag_count;
	// NOTE: A count of total (tagged) comments cannot be accurately done by summing tag_count, as comments in some subreddits may be counted multiple times.
	const char* cat_name;
	const char* tag_name;
	uint8_t r, g, b, a;
	QPieSeries* series = nullptr;
	if (this->chart != nullptr)
		delete this->chart;
	this->chart = new DonutBreakdownChart();
	int cat_r = 255;
	int cat_g = 0;
	int cat_b = 0;
	while(compsky::mysql::assign_next_row(_mysql::res1, &_mysql::row1, &cat_id, &cat_name, &tag_name, &tag_count, &r, &g, &b, &a)){
		if (cat_id != last_cat_id){
			if (series != nullptr){
				chart->addBreakdownSeries(series, QColor(cat_r, cat_g, cat_b, 255)); // TODO: Mix tag colours together to generate this colour, or allow the assignment of colours to categories themselves
				std::tie(cat_r, cat_g, cat_b) = std::make_tuple(cat_g, cat_b, cat_r);
			}
			series = new QPieSeries();
			series->setName(cat_name);
			last_cat_id = cat_id;
		}
		QPieSlice* slice = series->append(QString(tag_name), tag_count);
		slice->setColor(QColor(r, g, b, a));
	}
	this->chart->setAnimationOptions(QChart::AllAnimations);
	this->chart->setTitle("Total comment count per tag category");
	this->chart->legend()->setAlignment(Qt::AlignRight);
	
	QVBoxLayout* l = new QVBoxLayout;
	
	QChartView* chart_view = new QChartView(this->chart);
	chart_view->setRubberBand(QChartView::RectangleRubberBand);
	chart_view->setRenderHint(QPainter::Antialiasing);
	l->addWidget(chart_view);
	
	QPushButton* hide_this = new QPushButton("Hide", this);
	connect(hide_this, &QPushButton::clicked, this, &CatDoughnut::hide);
	l->addWidget(hide_this);
	
	this->setLayout(l);
	
	this->is_initialised = true;
}
