/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __VIEW_MATCHED_COMMENTS_H__
#define __VIEW_MATCHED_COMMENTS_H__

#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QWidget>

#include <compsky/mysql/mysql.hpp>


class ViewMatchedComments : public QWidget {
    Q_OBJECT
  public:
    explicit ViewMatchedComments(QWidget* parent);
    ~ViewMatchedComments();
  private:
    void next();
    void del_cmnt();
    void init();
    void toggle_order_btns();
    void view_matches();
    QLineEdit* tagname_input;
    QLineEdit* reasonname_input;
    QPlainTextEdit* textarea;
    QLabel* subname;
    QLabel* username;
    QLabel* reasonname;
    QLabel* datetime;
    QLineEdit* permalink;
    char post_id_str[10];
    char cmnt_id_str[10];
    char dt_buf[200];
    uint64_t cmnt_id;
    char* cmnt_body;
    size_t cmnt_body_sz;
    MYSQL_RES* res1;
    MYSQL_ROW row1;
    bool is_ascending;
};


#endif
