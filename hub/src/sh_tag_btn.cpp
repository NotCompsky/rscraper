/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */

#include "sh_tag_btn.hpp"

#include <QCompleter>
#include <QMessageBox>
#include <QStringList>

#include <compsky/mysql/query.hpp>

#include "categorytab.hpp"
#include "clbtn.hpp"
#include "name_dialog.hpp"
#include "notfound.hpp"
#include "tagnamelabel.hpp"


extern MYSQL_RES* RES1;
extern MYSQL_ROW ROW1;

extern QStringList category_names;


ShTagBtn::ShTagBtn(const uint64_t id,  QWidget* parent) : QPushButton("Share", parent), tag_id(id) {}

void ShTagBtn::exec(){
    NameDialog* namedialog = new NameDialog("Share with category", "");
    namedialog->name_edit->setCompleter(new QCompleter(category_names));
    
    const int rc = namedialog->exec();
    const QString qstr = namedialog->name_edit->text();
    
    delete namedialog;
    
    
    if (rc != QDialog::Accepted)
        return;
    if (qstr.isEmpty())
        return;
    
    if (!category_names.contains(qstr))
        return notfound::category(this, qstr);
    
    constexpr static const compsky::asciify::flag::Escape f_esc;
    
    compsky::mysql::query(&RES1, "SELECT t2c.tag_id FROM tag2category t2c, category c WHERE t2c.category_id=c.id AND t2c.tag_id=", this->tag_id, " AND c.name=\"", f_esc, '"', qstr, "\"");
    uint64_t prev_id = 0;
    while(compsky::mysql::assign_next_row(RES1, &ROW1, &prev_id));
    if (prev_id != 0){
        QMessageBox::information(this, "Ignored", "Tag already belongs to that category");
        return;
    }
    
    
    compsky::mysql::exec("INSERT IGNORE INTO tag2category SELECT ", this->tag_id, ", id FROM category WHERE name=\"", f_esc, '"', qstr, "\"");
    
    ClTagsTab* cat_tab = reinterpret_cast<ClTagsTab*>(this->parent());
    ClTagsTab* w = reinterpret_cast<ClTagsTab*>(cat_tab->tab_named(qstr)->widget());
    // There should be no need to test if w is nullptr, as we already test whether the name is in the category_names list.
    // If it does happen to fail, the issue is with not deleting old entries from category_names.
    const int indx = cat_tab->l->indexOf(this);
    
    int row, col, rowspan, colspan;
    cat_tab->l->getItemPosition(indx, &row, &col, &rowspan, &colspan);
    const TagNameLabel* tagnamelabel = reinterpret_cast<TagNameLabel*>(cat_tab->l->itemAtPosition(row, 0)->widget());
    const QString tagname = tagnamelabel->text();
    
    const SelectColourButton* clbtn = reinterpret_cast<SelectColourButton*>(cat_tab->l->itemAtPosition(row, 1)->widget());
    const QColor cl = clbtn->colour;
    
    w->add_tag_row(this->tag_id, tagname, cl); // TODO: Copy tab colour to the new category
}

void ShTagBtn::mousePressEvent(QMouseEvent* e){
    switch(e->button()){
        case Qt::LeftButton:
            return this->exec();
        default: return;
    }
}
