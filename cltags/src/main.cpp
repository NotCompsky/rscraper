#include <QApplication>
#include "cltagswindow.h"

int main(int argc,  char** argv){
    QApplication app(argc, argv);
    ClTagsDialog win(argv[1]);
    win.show();
    return app.exec();
}
