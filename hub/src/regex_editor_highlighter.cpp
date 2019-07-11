#include "regex_editor_highlighter.hpp"

#include <QRegularExpression>
#include <QTextCharFormat>


struct HighlightingRule {
	QRegularExpression pattern;
	QTextCharFormat    format;
	HighlightingRule(const QRegularExpression& re) : pattern(re), format() {}
};


#define n_highlighting_rules 6
static HighlightingRule highlighting_rules[n_highlighting_rules] = {
	QRegularExpression("(?:^|[^\\\\])[(]([?]P<[^>]*>)"),// Capture group name (outer) // TODO: Individual formatting of multiple capture groups
	QRegularExpression("(?:^|[^\\\\])[(][?]P<([^>]*)>"),// Capture group name (inner)
	QRegularExpression("(?:^|[^\\\\])([()])"),          // Capture group bracket // NOTE: [(] is a false positive; left and right brackets are not paired up
	QRegularExpression("([ \t]+#.*)"),                  // Comment
	QRegularExpression("(\\\\[\\\\nrtv])"),             // Escape sequence parsed by the hub's pre-processor
	QRegularExpression("(?:^|[^\\\\]\n)([ \t]*)") // Whitespace after newline that is ignored by the hub's pre-processor // NOTE: Fails to not comment out lines preceded by a '\' - newlines work weirdly. // TODO: Fix this.
};

static const QColor cl_comment(0, 255, 0, 70);


RegexEditorHighlighter::RegexEditorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    highlighting_rules[0].format.setForeground(Qt::darkBlue);
    highlighting_rules[1].format.setForeground(Qt::darkBlue);
    highlighting_rules[1].format.setFontWeight(QFont::Bold);
    highlighting_rules[2].format.setForeground(Qt::blue);
    highlighting_rules[2].format.setFontWeight(QFont::Bold);
    highlighting_rules[3].format.setBackground(cl_comment);
    highlighting_rules[4].format.setForeground(Qt::red);
    highlighting_rules[5].format.setBackground(cl_comment);
}

void RegexEditorHighlighter::highlightBlock(const QString& text) {
    for (auto i = 0;  i < n_highlighting_rules;  ++i) {
        const HighlightingRule rule = highlighting_rules[i];
        QRegularExpressionMatchIterator match_itr = rule.pattern.globalMatch(text);
        while (match_itr.hasNext()) {
            QRegularExpressionMatch match = match_itr.next();
            setFormat(match.capturedStart(1), match.capturedLength(1), rule.format);
        }
    }
}
