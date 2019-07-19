/*
 * Based on example given in Qt documentation: https://doc.qt.io/archives/qt-5.11/qtcharts-donutbreakdown-example.html
 */

#include "tag_pie.hpp"

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


TagPie::TagPie(const int tag_id,  QWidget* parent) : QDialog(parent) {
	using namespace QtCharts;
	
	// TODO: Use worker thread for this
	compsky::mysql::query(&RES1,  "SELECT s.name, COUNT(u2scc.count) as c FROM subreddit s, subreddit2tag s2t, user2subreddit_cmnt_count u2scc WHERE s2t.tag_id=", tag_id, " AND s.id=s2t.subreddit_id AND s2t.subreddit_id=u2scc.subreddit_id GROUP BY s.name ORDER BY c DESC");
	uint64_t count;
	char* subreddit_name;
	uint8_t r, g, b, a;
	int subreddit_count = 0;
	uint64_t misc_count = 0;
	QPieSeries* series = new QPieSeries();
	QChart* chart = new QChart();
	while(compsky::mysql::assign_next_row(RES1, &ROW1, &subreddit_name, &count)){
		if (++subreddit_count < 30)
			series->append(QString("%1: %2").arg(subreddit_name).arg(count), count);
		else
			misc_count += count;
	}
	if (misc_count != 0)
		series->append(QString("%1 misc subreddits: %2").arg(subreddit_count-30).arg(misc_count), misc_count);
	series->setLabelsVisible();
	chart->addSeries(series);
	for (QLegendMarker* marker : chart->legend()->markers(series)){
		// This loop is adapted from the Breakdown Chart example in Qt5 documentation
		QPieLegendMarker* pie_marker = qobject_cast<QPieLegendMarker*>(marker);
		pie_marker->setLabel(pie_marker->slice()->label());
		pie_marker->setFont(QFont("Arial", 8));
	}
	
	chart->setAnimationOptions(QChart::AllAnimations);
	chart->setTitle("Total comment count per tag");
	chart->legend()->setAlignment(Qt::AlignRight);
	
	QVBoxLayout* l = new QVBoxLayout;
	
	QChartView* chart_view = new QChartView(chart);
	chart_view->setRubberBand(QChartView::RectangleRubberBand);
	chart_view->setRenderHint(QPainter::Antialiasing);
	l->addWidget(chart_view);
	
	QPushButton* hide_this = new QPushButton("Hide", this);
	connect(hide_this, &QPushButton::clicked, this, &TagPie::hide);
	l->addWidget(hide_this);
	
	this->setLayout(l);
	
	this->show();
}
