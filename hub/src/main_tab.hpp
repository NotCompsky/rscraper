/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef RSCRAPER_HUB_MAIN_TAB_HPP
#define RSCRAPER_HUB_MAIN_TAB_HPP

#include "cat_doughnut.hpp"

#include <QTabWidget>
#include <QWidget>


class MainTab : public QWidget {
	Q_OBJECT
  public:
	explicit MainTab(QTabWidget* tab_widget,  QWidget* parent = 0);
  public Q_SLOTS:
	void add_category();
  private:
	QTabWidget* tab_widget;
	CatDoughnut* cat_doughnut;
};

#endif
