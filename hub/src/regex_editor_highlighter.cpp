#include "regex_editor_highlighter.hpp"

#include <QRegularExpression>
#include <QTextCharFormat>


#define n_highlighting_rules 8
static const QRegularExpression highlighting_regex(
	"(?<!\\\\)(?:"
		"([(])([?]P<([^>]*)>)?|"	// Capture group opening bracket (#1), and optionally name (inner: #3, outer: #2)
		"([)])"				// Capture group closing bracket (#4) // NOTE: [(] is a false positive; left and right brackets are not paired up
	")|"
	"((?:^|[ \t]+)#.*)|"			// Comment (#5)
	"(\\\\[\\\\nrtv])|"			// Escape sequence parsed by the hub's pre-processor (#6) // TODO: maybe ignore if preceded by an odd number of escape characters
	"(?<!\\\\)(?<=^)([ \t]*)"		// Whitespace after newline that is ignored by the hub's pre-processor (#7) // NOTE: Fails to not comment out lines preceded by an escape character - newlines work weirdly. // TODO: Fix this.
);

static QTextCharFormat highlighting_fmts[n_highlighting_rules];

static const QColor cl_comment(0, 255, 0, 70);


RegexEditorHighlighter::RegexEditorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    highlighting_fmts[1].setForeground(Qt::blue);	// Capture group opening bracket
    highlighting_fmts[1].setFontWeight(QFont::Bold);	// Capture group opening bracket
    highlighting_fmts[2].setForeground(Qt::darkBlue);
    highlighting_fmts[3].setForeground(Qt::darkBlue);
    highlighting_fmts[3].setFontWeight(QFont::Bold);
    highlighting_fmts[4].setForeground(Qt::blue);	// Capture group closing bracket
    highlighting_fmts[4].setFontWeight(QFont::Bold);	// Capture group closing bracket
    highlighting_fmts[5].setBackground(cl_comment);
    highlighting_fmts[6].setForeground(Qt::red);
    highlighting_fmts[7].setBackground(cl_comment);
}

void RegexEditorHighlighter::highlightBlock(const QString& text) {
    QRegularExpressionMatchIterator match_itr = highlighting_regex.globalMatch(text);
    while (match_itr.hasNext()) {
        QRegularExpressionMatch match = match_itr.next();
        for (auto i = 1;  i < n_highlighting_rules;  ++i) {
            setFormat(match.capturedStart(i), match.capturedLength(i), highlighting_fmts[i]);
        }
    }
}
