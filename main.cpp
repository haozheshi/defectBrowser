#include "defectbrowser.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DefectBrowser w;
    batchloader b;
    w.setStatusBar(nullptr);
    w.setBatchLoader(&b);
    Qt::WindowFlags flags = 0;
       flags |= Qt::WindowMinimizeButtonHint;
       flags |= Qt::WindowCloseButtonHint;
   QObject::connect(&b, &batchloader::openBatch,
               &w, &DefectBrowser::on_OpenBatch);
    while(b.exec() != QDialog::Accepted)
    {

    }
    w.setWindowFlags(flags);
    w.showMaximized();
    return a.exec();
}
