TARGET  =   rscraper-hub
QT += charts widgets
SOURCES =   src/main.cpp \
            src/mainwindow.cpp \
            src/maintab.cpp \
            src/wlbl_label.cpp \
            src/wlbl_reasonwise_label.cpp \
            src/view_matched_comments.cpp \
            ../utils/src/id2str.cpp \
            src/name_dialog.cpp \
            src/msgbox.cpp \
            src/notfound.cpp \
            src/3rdparty/donutbreakdownchart.cpp \
            src/3rdparty/mainslice.cpp \
            src/categorytab.cpp \
            src/clbtn.cpp \
            src/tagnamelabel.cpp \
            src/add_sub2tag_btn.cpp \
            src/rm_tag_btn.cpp \
            src/regex_editor.cpp \
            src/cat_doughnut.cpp
INCLUDEPATH += ../utils/src $COMPSKY_INCLUDE_DIRS
LIBS += -lmysqlclient -lcompsky_asciify -lcompsky_mysql -lboost_regex
