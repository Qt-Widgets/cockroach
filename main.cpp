/*
 * File:   main.cpp
 * Author: Ivan
 *
 * Created on 24 Октябрь 2011 г., 14:56
 */

#include <QApplication>
#include "MainForm.h"

int main(int argc, char *argv[]) {
    // initialize resources, if needed
    // Q_INIT_RESOURCE(resfile);
    
    QApplication app(argc, argv);

    // create and show your widgets here
    MainForm mainForm;
    mainForm.show();
    qApp->setStyle("cleanlooks");
    qApp->setPalette(qApp->style()->standardPalette());
    return app.exec();
}
