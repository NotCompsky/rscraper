#include "regex_editor_highlighter.hpp"

#include <QRegularExpression>
#include <QTextCharFormat>


struct HighlightingRule {
	QRegularExpression pattern;
	QTextCharFormat    format;
	HighlightingRule(const QRegularExpression& re) : pattern(re), format() {}
};


#define n_highlighting_rules 3
static HighlightingRule highlighting_rules[3] = {
	QRegularExpression("(?:^|[^\\\\])[(][?]P<([^>]*)>"),// Capture group name
	QRegularExpression("(?:^|[^\\\\])([()])"),          // Capture group bracket // NOTE: [(] is a false positive; left and right brackets are not paired up
	QRegularExpression("[ \t](#[^\n]*)")          // Comment
};


RegexEditorHighlighter::RegexEditorHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    highlighting_rules[0].format.setForeground(Qt::darkBlue);
    highlighting_rules[0].format.setFontWeight(QFont::Bold);
    highlighting_rules[1].format.setForeground(Qt::blue);
    highlighting_rules[1].format.setFontWeight(QFont::Bold);
    highlighting_rules[2].format.setForeground(Qt::green);
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
