#ifndef DEFECTBROWSER_H
#define DEFECTBROWSER_H

#include <QMainWindow>
#include "defectfilterbook.h"
#include "defectpagepage.h"
#include "pageitempicture.h"
#include "bookscrollbar.h"
#include <QTabWidget>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include "pcie.h"
#include "batchloader.h"
#include "statistictable.h"
QT_BEGIN_NAMESPACE
namespace Ui { class DefectBrowser; }
QT_END_NAMESPACE
class DefectBrowser : public QMainWindow
{
    Q_OBJECT

public:
    DefectBrowser(QWidget *parent = nullptr);
    ~DefectBrowser();
    void setBatchLoader(batchloader* loader){m_loader = loader;}
signals:
    void openDBDone(QStringList levelList, QStringList defectList);
private slots:
    void on_FlipPage(int mode);
    void on_ChangeQueryLang();
    void on_updateDateTime();
    void on_OpenDBDone(QStringList levelList, QStringList defectList);
    void on_pushButton_2_clicked();

public slots:
    void on_OpenBatch(QString source);
private:
    Ui::DefectBrowser *ui;
    defectFilterBook* m_book;
    defectPage* m_page;
    pageItemPicture* m_pagePicture;
    bookScrollBar* m_scroll;
private:
    bool changBatchFinish = true;
    QDateTime m_startDate;
    QDateTime m_endDate;
    batchloader* m_loader;
};
#endif // DEFECTBROWSER_H
