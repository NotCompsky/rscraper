/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#ifndef __REGEX_EDITOR_H__
#define __REGEX_EDITOR_H__

#include <QDialog>
#include <QPlainTextEdit>


class RegexEditor : public QDialog {
    Q_OBJECT
  public:
    RegexEditor(const QString& human_fp,  const QString& raw_fp,  QWidget* parent = 0);
  private Q_SLOTS:
    void test_regex();
    void save_to_file();
  private:
    const QString to_final_format();
    void load_file();
    QFile f_human;
    QFile f_raw;
    QPlainTextEdit* text_editor;
};


#endif
