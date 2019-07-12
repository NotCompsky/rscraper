#include "regex_editor_highlighter.hpp"

#include <QRegularExpression>
#include <QTextCharFormat>


#define n_highlighting_rules 12
static const QRegularExpression highlighting_regex(
	"((?:^|[ \t]+)#.*)|"			// Comment
	"(?<!\\\\)((?:\\\\\\\\)*)(?:"			// Allow an even number of escape characters before (#1)
		"([({])(?:"			// (Capture group or var declaration) opening bracket
			"([?]P<([^>]*)>)|"	// (Capture group or var declaration) name (inner and outer)
			"(\\?:)"		// Non-capturing group declaration
		")?|"
		"([)}])|"			// (Capture group or var declaration) closing bracket (#5) // NOTE: [(] is a false positive; left and right brackets are not paired up
		"(?<=^)([ \t]*)|"		// Whitespace after newline that is ignored by the hub's pre-processor (#6) // NOTE: Fails to not comment out lines preceded by an escape character - newlines work weirdly. // TODO: Fix this.
		"(\\$\\{[^}]+\\})|"		// Variable substitution (#7) (not implemented into regex pre-processor yet, but planned)
		"(\\[[^]]+\\])|"		// Square bracket set
		"(\\|)"				// OR operator
	")|"
	"(\\\\[\\\\nrtv])"			// Escape sequence parsed by the hub's pre-processor (#9) // TODO: maybe ignore if preceded by an odd number of escape characters
);

static QTextCharFormat highlighting_fmts[n_highlighting_rules+1];

static const QColor cl_comment(0, 255, 0, 70);
static const QColor cl_varsub(0, 0, 255, 70);


RegexEditorHighlighter::RegexEditorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    int i = 0;
    highlighting_fmts[++i].setBackground(cl_comment);	// Comment
    highlighting_fmts[++i].setForeground(Qt::red);	// Escape characters (even number preceding)
    highlighting_fmts[++i].setForeground(Qt::blue);	// Capture group opening bracket
    highlighting_fmts[ i ].setFontWeight(QFont::Bold);	// Capture group opening bracket
    highlighting_fmts[++i].setForeground(Qt::darkBlue);	// Capture group name (outer)
    highlighting_fmts[++i].setForeground(Qt::darkBlue);	// Capture group name (inner)
    highlighting_fmts[ i ].setFontWeight(QFont::Bold);	// Capture group name (inner)
    highlighting_fmts[++i].setForeground(Qt::gray);	// Non-capturing group declaration
    highlighting_fmts[++i].setForeground(Qt::blue);	// Capture group closing bracket
    highlighting_fmts[ i ].setFontWeight(QFont::Bold);	// Capture group closing bracket
    highlighting_fmts[++i].setBackground(cl_comment);	// Comment
    highlighting_fmts[++i].setForeground(Qt::yellow);	// Variable substitution
    highlighting_fmts[ i ].setBackground(cl_varsub);	// Variable substitution
    highlighting_fmts[ i ].setFontWeight(QFont::Bold);	// Variable substitution
    highlighting_fmts[++i].setFontWeight(QFont::Light);	// Square bracket set
    highlighting_fmts[++i].setFontWeight(QFont::Bold);	// OR operator
    highlighting_fmts[++i].setForeground(Qt::red);	// Escape characters parsed by pre-processor


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
