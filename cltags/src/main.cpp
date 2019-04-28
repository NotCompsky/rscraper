#include <QApplication>
#include "cltagswindow.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    ClTagsDialog player;
    player.show();
    return a.exec();
}
