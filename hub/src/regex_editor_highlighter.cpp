#include "regex_editor_highlighter.hpp"

#include <QRegularExpression>
#include <QTextCharFormat>


#define n_highlighting_rules 9
static const QRegularExpression highlighting_regex(
	"(?<!\\\\)((?:\\\\\\\\)*)(?:"			// Allow an even number of escape characters before (#1)
		"([({])([?]P<([^>]*)>)?|"	// (Capture group or var declaration) opening bracket (#2), and optionally name (inner: #4, outer: #3)
		"([)}])|"			// (Capture group or var declaration) closing bracket (#5) // NOTE: [(] is a false positive; left and right brackets are not paired up
		"(?<=^)([ \t]*)|"		// Whitespace after newline that is ignored by the hub's pre-processor (#6) // NOTE: Fails to not comment out lines preceded by an escape character - newlines work weirdly. // TODO: Fix this.
		"(\\$\\{[^}]+\\})"		// Variable substitution (#7) (not implemented into regex pre-processor yet, but planned)
	")|"
	"((?:^|[ \t]+)#.*)|"			// Comment (#8)
	"(\\\\[\\\\nrtv])"			// Escape sequence parsed by the hub's pre-processor (#9) // TODO: maybe ignore if preceded by an odd number of escape characters
);

static QTextCharFormat highlighting_fmts[n_highlighting_rules+1];

static const QColor cl_comment(0, 255, 0, 70);
static const QColor cl_varsub(0, 0, 255, 70);


RegexEditorHighlighter::RegexEditorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    highlighting_fmts[1].setForeground(Qt::red);	// Escape characters (even number preceding)
    highlighting_fmts[2].setForeground(Qt::blue);	// Capture group opening bracket
    highlighting_fmts[2].setFontWeight(QFont::Bold);	// Capture group opening bracket
    highlighting_fmts[3].setForeground(Qt::darkBlue);	// Capture group name (outer)
    highlighting_fmts[4].setForeground(Qt::darkBlue);	// Capture group name (inner)
    highlighting_fmts[4].setFontWeight(QFont::Bold);	// Capture group name (inner)
    highlighting_fmts[5].setForeground(Qt::blue);	// Capture group closing bracket
    highlighting_fmts[5].setFontWeight(QFont::Bold);	// Capture group closing bracket
    highlighting_fmts[8].setBackground(cl_comment);	// Comment
    highlighting_fmts[9].setForeground(Qt::red);	// Escape characters parsed by pre-processor
    highlighting_fmts[6].setBackground(cl_comment);	// Comment
    highlighting_fmts[7].setForeground(Qt::yellow);	// Variable substitution
    highlighting_fmts[7].setBackground(cl_varsub);	// Variable substitution
    highlighting_fmts[7].setFontWeight(QFont::Bold);	// Variable substitution
    // NOTE: background highlights are used to indicate that the length of the resulting regex would be modified - either by removing whitespace, or pasting text with variable substitution.
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
