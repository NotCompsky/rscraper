/*
 * rscraper Copyright (C) 2019 Adam Gray
 * This program is licensed with GPLv3.0 and comes with absolutely no warranty.
 * This code may be copied, modified, distributed etc. with accordance to the GPLv3.0 license (a copy of which is in the root project directory) under the following conditions:
 *     This copyright notice must be included at the beginning of any copied/modified file originating from this project, or at the beginning of any section of code that originates from this project.
 */


#include <QApplication>
#include "cltagswindow.h"

int main(int argc,  char** argv){
    QApplication app(argc, argv);
    
    ClTagsDialog* win = new ClTagsDialog();
    win->show();
    
    return app.exec();
}
