// Entirely from Qt5 docs: https://doc.qt.io/qt-5/qtwidgets-widgets-codeeditor-example.html 
 
/* 
Copyright (C) 2018 The Qt Company Ltd. 
 
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are 
met: 
 * Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer. 
 * Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in 
   the documentation and/or other materials provided with the 
   distribution. 
 * Neither the name of The Qt Company Ltd nor the names of its 
   contributors may be used to endorse or promote products derived 
   from this software without specific prior written permission. 
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


#include "codeeditor.hpp"

#include "linenumberarea.hpp"

#include <QTextCharFormat>
#include <QPainter>
#include <QTextBlock>


static QTextCharFormat fmt_bracket;


CodeEditor::CodeEditor(QWidget *parent)
	: QPlainTextEdit(parent)
	, other_bracket(-1)
{
	fmt_bracket.setBackground(Qt::black);
	fmt_bracket.setForeground(Qt::yellow);
	fmt_bracket.setFontWeight(QFont::Bold);
	
	lineNumberArea = new LineNumberArea(this);

	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
	connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
	connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);
	connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlight_brackets);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();
}


int CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}

	int space = 3 + digits*
# if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
	fontMetrics().width(QLatin1Char('g'));
# else
	fontMetrics().horizontalAdvance(QLatin1Char('9'));
# endif
	
	return space;
}


void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}


void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
	if (dy)
		lineNumberArea->scroll(0, dy);
	else
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

	if (rect.contains(viewport()->rect()))
		updateLineNumberAreaWidth(0);
}


void CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}


void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

		QColor lineColor = QColor(Qt::yellow).lighter(160);

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}


void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
							 Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

bool is_even_number_of_escapes(const QString& q,  int pos){
	unsigned int n = 0;
	while(q.at(--pos) == QChar('\\'))
		++n;
	return (n % 2  ==  0);
}

void fmt_char(const QTextCharFormat& fmt,  QTextCursor cursor,  const int pos){
	cursor.setPosition(pos);
	cursor.movePosition(QTextCursor::Right,  QTextCursor::KeepAnchor,  1);
	cursor.setCharFormat(fmt);
}

QTextCharFormat fmt_char_retprev(const QTextCharFormat& fmt,  QTextCursor cursor,  const int pos){
	cursor.setPosition(pos);
	cursor.movePosition(QTextCursor::Right,  QTextCursor::KeepAnchor,  1);
	const QTextCharFormat fmt_orig = cursor.charFormat();
	cursor.setCharFormat(fmt);
	return fmt_orig;
}

int CodeEditor::pos_of_partner() const {
	const int pos_init = this->textCursor().position();
	int pos = pos_init;
	const QString q = this->toPlainText();
	
	if (pos == q.size())
		// We selected the space at the end of the file - after any characters
		return -1;
	
	const QChar a = q.at(pos);
	QChar b;
	int increment = 1; // forwards
	if (a == QChar('('))
		b = QChar(')');
	else if (a == QChar(')')){
		b = QChar('(');
		increment = -1;
	} else if (a == QChar('{'))
		b = QChar('}');
	else if (a == QChar('}')){
		b = QChar('{');
		increment = -1;
	} else return -1;
	
	int depth = 0;
	while(true){
		/* Quit if no corresponding bracket was found */
		if (increment == -1){
			if (pos == 0){
				return -1;
			}
		} else {
			if (pos == q.size() - 1)
				return -1;
		}
		
		pos += increment;
		const QChar c = q.at(pos);
		if (c == b){
			if (depth == 0)
				break;
			else
				if (is_even_number_of_escapes(q, pos))
					--depth;
		} else if (c == a)
			if (is_even_number_of_escapes(q, pos))
				++depth;
	}
	
	return pos;
}

void CodeEditor::jump_to_partner(){
	QTextCursor cursor = this->textCursor();
	const int pos = this->pos_of_partner();
	if (pos == -1)
		return;
	cursor.setPosition(pos);
	this->setTextCursor(cursor);
}

void CodeEditor::highlight_brackets(){
	QTextCursor cursor = this->textCursor();
	if (this->other_bracket != -1){
		fmt_char(this->other_bracket_fmt, cursor, other_bracket);
	}
	
	this->other_bracket = this->pos_of_partner();
	if (this->other_bracket == -1)
		return;
	
	this->other_bracket_fmt = fmt_char_retprev(fmt_bracket, cursor, this->other_bracket);
	
	/*
	cursor.setPosition(pos);
	const auto f  = (increment == 1)  ?  QTextCursor::Left  :  QTextCursor::Right;
	const int len = (increment == 1)  ?  pos - pos_init     :  pos_init - pos;
	cursor.movePosition(f,  QTextCursor::KeepAnchor,  len + 1);
	this->text_editor->setTextCursor(cursor);
	*/
}
