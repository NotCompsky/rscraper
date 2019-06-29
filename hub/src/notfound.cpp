#include "notfound.hpp"

#include <QMessageBox>


namespace notfound {

void category(QWidget* parent,  const QString& s){
    QMessageBox::information(parent, "Category Not Found", "No category named `" + s + "` was found in the database.\nIf you are sure the entry exists, restart this program so that the caches are updated.");
}

void subreddit(QWidget* parent,  const QString& s){
    QMessageBox::information(parent, "Subreddit Not Found", "No subreddit named `" + s + "` was found in the database.\nHint: Use `rscrape-cmnts` and/or `rscraper-import user.csv` to populate the table.\nIf you are sure the entry exists, restart this program so that the caches are updated.");
    return;
}

void reason(QWidget* parent,  const QString& s){
    QMessageBox::information(parent, "Reason Not Found", "No reason named `" + s + "` was found in the database.\nHint: Use `rscrape-cmnts` and/or `rscraper-import reason_matched.csv` to populate the table - for instance, rscrape-cmnts populates the reason_matched table based on the RSCRAPER_REGEX_FILE (the comment body regexp).\nIf you are sure the entry exists, restart this program so that the caches are updated.");
}

void user(QWidget* parent,  const QString& s){
    QMessageBox::information(parent, "Reason Not Found", "No user named `" + s + "` was found in the database.\nHint: Use `rscrape-cmnts` and/or `rscraper-import user.csv` to populate the table - for instance, rscrape-cmnts populates the user table based on which comments match the regex in RSCRAPER_REGEX_FILE (the comment body regexp).\nIf you are sure the entry exists, restart this program so that the caches are updated.");
}

}
