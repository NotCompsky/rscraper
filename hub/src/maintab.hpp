/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __MAINTAB_H__
#define __MAINTAB_H__

#include <QGridLayout>
#include <QTabWidget>
#include <QWidget>


class CatDoughnut;

class MainTab;
typedef  void (MainTab::*MainTabMemberFnct)();


class MainTab : public QWidget {
    Q_OBJECT
  public:
    explicit MainTab(QTabWidget* tab_widget,  QWidget* parent = 0);
  public Q_SLOTS:
    void add_category();
  private:
    int add(MainTabMemberFnct f_add,  MainTabMemberFnct f_rm,  QGridLayout* l,  int row);
    
    void open_cmnt_body_re_editor();
    
    void add_subreddit_to(const char* tblname);
    void rm_subreddit_from(const char* tblname);
    
    void add_subreddit_to_reason(const char* tblname);
    void rm_subreddit_from_reason(const char* tblname);
    
    void add_user_to(const char* tblname);
    void rm_user_from(const char* tblname);
    
    void add_to_subreddit_count_bl();
    void rm_from_subreddit_count_bl();
    
    void add_to_user_count_bl();
    void rm_from_user_count_bl();
    
    void add_to_subreddit_contents_wl();
    void rm_from_subreddit_contents_wl();
    void add_to_subreddit_contents_bl();
    void rm_from_subreddit_contents_bl();
    
    void add_to_user_contents_wl();
    void rm_from_user_contents_wl();
    void add_to_user_contents_bl();
    void rm_from_user_contents_bl();
    
    void add_to_reason_subreddit_wl();
    void rm_from_reason_subreddit_wl();
    
    void add_to_reason_subreddit_bl();
    void rm_from_reason_subreddit_bl();
    
    QTabWidget* tab_widget;
    CatDoughnut* cat_doughnut;
};

#endif
