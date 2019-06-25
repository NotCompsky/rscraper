#ifndef __NOTFOUND_H__
#define __NOTFOUND_H__

#include <QString>
#include <QWidget>


namespace notfound {

void subreddit(QWidget* parent,  const QString& s);
void reason(QWidget* parent,  const QString& s);
void user(QWidget* parent,  const QString& s);

}

#endif
