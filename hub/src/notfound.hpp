#ifndef RSCRAPER_HUB_NOTFOUND_HPP
#define RSCRAPER_HUB_NOTFOUND_HPP

#include <QString>
#include <QWidget>


namespace notfound {

void category(QWidget* parent,  const QString& s);
void subreddit(QWidget* parent,  const QString& s);
void reason(QWidget* parent,  const QString& s);
void user(QWidget* parent,  const QString& s);

}

#endif
