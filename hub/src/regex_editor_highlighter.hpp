/*
Based on the example from the official Qt docs: https://doc.qt.io/qt-5/qtwidgets-richtext-syntaxhighlighter-example.html
*/

#include <QSyntaxHighlighter>
#include <QTextDocument>


class RegexEditorHighlighter : public QSyntaxHighlighter {
	Q_OBJECT
	
  public:
	RegexEditorHighlighter(QTextDocument* parent = 0);
	
  protected:
	void highlightBlock(const QString& text) override;
};
