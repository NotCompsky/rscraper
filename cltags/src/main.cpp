#include <QApplication>
#include "cltagswindow.h"

int main(int argc, char *argv[]){
    QApplication a(argc, argv);
    ClTagsDialog player(argv[1]);
    player.show();
    return a.exec();
}
