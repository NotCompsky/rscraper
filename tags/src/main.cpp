#include <QApplication>
#include "cltagswindow.h"

int main(int argc,  char** argv){
    QApplication app(argc, argv);
    
    ClTagsDialog* win = new ClTagsDialog();
    win->show();
    
    return app.exec();
}
