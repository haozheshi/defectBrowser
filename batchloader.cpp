#include "batchloader.h"
#include "ui_batchloader.h"
#include <QScreen>
#include <QSettings>
#include <QTextCodec>
#include <QDir>
#include <QFileDialog>

inline QStringList getdbList(QString path)
{
    QDir dir;
    QStringList nameFilters;
    nameFilters << "*.db";
    dir.setNameFilters(nameFilters);
    dir.setPath(path);
    return dir.entryList();
}



batchloader::batchloader(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::batchloader)
{
    ui->setupUi(this);
    QString homePath = QCoreApplication::applicationDirPath();
    QSettings ini(QString("%1/DefectBrowser.ini").arg(homePath), QSettings::IniFormat);
    ini.setIniCodec(QTextCodec::codecForName("UTF-8"));
    ini.beginGroup("Path");
    m_strRoot = ini.value("Batch").toString();
    ini.endGroup();
    QStringList dblist = getdbList(m_strRoot);
    model.setStringList(dblist);
    ui->listView->setModel(&model);
    connect(ui->listView, &QListView::clicked,
            this, &batchloader::on_openBatch_clicked);
}

void batchloader::on_openBatch_clicked(const QModelIndex &index)
{
    if(index.isValid())
    {
        QString file = model.data(index).toString();
        m_strBatchPath = m_strRoot + "//" + file;
    }
}


batchloader::~batchloader()
{
    delete ui;
}

void batchloader::on_buttonBox_accepted()
{
    emit openBatch(m_strBatchPath);
}
