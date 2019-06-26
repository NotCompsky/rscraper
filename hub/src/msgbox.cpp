/*
 * Mainly to simply have PlainText as the default, but also to allow larger window widths.
 */

#include "msgbox.hpp"

MsgBox::MsgBox(QWidget* parent,  const QString& text,  const QString& details,  int w) : QMessageBox(parent), w(w) {
    this->setTextFormat(Qt::PlainText);
    this->setText(text);
    this->setWindowModality(Qt::NonModal);
    this->setDetailedText(details);
}
void MsgBox::showEvent(QShowEvent* e){
    QMessageBox::showEvent(e);
    if (this->w)
        this->setMinimumWidth(this->w);
}

void MsgBox::resizeEvent(QResizeEvent* e){
    QMessageBox::resizeEvent(e);
    if (this->w)
        this->setMinimumWidth(this->w);
}
