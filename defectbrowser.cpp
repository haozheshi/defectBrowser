#include "defectbrowser.h"
#include "ui_defectbrowser.h"
#include <QAbstractScrollArea>
#include "PictureDelegate.h"
#include <QDebug>
#include <QTime>
#include <QThread>
#include <codecvt>
#include <QToolTip>

inline std::string keyValueQuery(sqlite3* db, std::string command)
{
    std::string resultStr;
    sqlite3_stmt* stmt = nullptr;
    sqlite3_prepare_v2(db, command.c_str(), int(command.length()), &stmt, NULL);
    if(stmt != nullptr)
    {
        int ret = sqlite3_step(stmt);
        if(ret == SQLITE_ROW)
        {
            const unsigned char* result = sqlite3_column_text(stmt, 0);
            int nBytes = sqlite3_column_bytes(stmt, 0);
            resultStr = std::string((char*)result, nBytes);
        }
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
    return resultStr;
}

DefectBrowser::DefectBrowser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DefectBrowser)
{
    ui->setupUi(this);
    ui->menubar->setVisible(false);
    //set model
    m_book = new defectFilterBook();
    m_page = new defectPage(m_book, this);
    m_pagePicture = new pageItemPicture(this);
    m_pagePicture->setSourceModel(m_page);
    ui->listView->setModel(m_pagePicture);
    ui->listView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    connect(m_book, &defectFilterBook::contentsChange, m_page,
            &defectPage::updateWords);
    ui->listView->setSpacing(12);
    ui->listView->setResizeMode(QListView::Adjust);
    ui->listView->setFlow(QListView::LeftToRight);
    ui->listView->setWrapping(true);
    ui->listView->setItemDelegate(new PictureDelegate(this));
    //ui->listView->setFixedWidth(856);
    //set scroll bar
    m_scroll = new bookScrollBar(Qt::Orientation::Vertical, this);
    ((QAbstractScrollArea*)ui->listView)->setVerticalScrollBar(m_scroll);
    connect(m_scroll, &bookScrollBar::FlipPage, this, &DefectBrowser::on_FlipPage);
    //set slider event
    connect(ui->horizontalSlider, &QSlider::valueChanged, this,
            &DefectBrowser::on_updateDateTime);
    connect(ui->horizontalSlider_2, &QSlider::valueChanged, this,
            &DefectBrowser::on_updateDateTime);
    //batch
    QWidget* batchContainer = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout();
    batchContainer->setLayout(layout);

    //设置查询按钮
    this->ui->pushButton->setToolTip(tr("search"));
    this->ui->label_2->setToolTip(tr("prductLevel"));
    this->ui->label_3->setToolTip(tr("defectType"));

    //设置查询控件的回调函数
    qRegisterMetaType<QList<QList<statisticData>>>("QList<QList<statisticData>>");
    connect(this->ui->pushButton, &QPushButton::clicked,
            this, &DefectBrowser::on_ChangeQueryLang);
    connect(this, &DefectBrowser::openDBDone, this,
            &DefectBrowser::on_OpenDBDone);
}

void DefectBrowser::on_FlipPage(int mode)
{

    if(FlipMode::NextPage == mode)
    {
        if(m_book->flipBook(FlipMode::NextPage))
        {
            ui->listView->scrollToTop();
        }
    }
    else
    {
        if(m_book->flipBook(FlipMode::PrevPage))
        {
            ui->listView->scrollToBottom();
        }
    }
    m_scroll->resetFlip();
}

void DefectBrowser::on_OpenBatch(QString strBatchPath)
{
    if(!changBatchFinish)
    {
        QMessageBox::information(this, "information", "last batch not close");
        return;
    }
    changBatchFinish = false;
    //清空候选框
    ui->comboBox->clear();
    ui->comboBox_2->clear();
    std::wstring source = strBatchPath.toStdWString();
    auto fun = [=](){
        m_book->setSource(source.c_str());
        QStringList batchlist = m_book->getShelfList();
        m_book->setShelfName("defectTable");
        //重置时间
        m_startDate = m_book->getFirstPageDate();
        m_endDate = m_book->getLastPageDate();
        int hspan = 0;
        if(m_startDate.date().day() ==  m_endDate.date().day())
        {
           hspan = m_endDate.time().hour() - m_startDate.time().hour() + 1;
        }
        else
        {
           hspan = 24 - m_startDate.time().hour() + m_endDate.time().hour() + 1;
        }
        m_endDate = m_startDate.addSecs(hspan * 60 * 60);
        ui->horizontalSlider->setRange(0, hspan);
        ui->horizontalSlider->setSingleStep(1);
        ui->horizontalSlider->setTickInterval(1);
        ui->horizontalSlider->setValue(0);
        ui->horizontalSlider_2->setRange(0, hspan);
        ui->horizontalSlider_2->setSingleStep(1);
        ui->horizontalSlider_2->setTickInterval(1);
        ui->horizontalSlider_2->setValue(hspan);
        ui->label_6->setText(m_startDate.toString());
        ui->label_7->setText(m_endDate.toString());
        //获取产品等级
        QStringList levelList = m_book->getCatagoryList("quality");
        //获取缺陷类型
        QStringList defectList = m_book->getCatagoryList("type");
        levelList.sort();
        levelList.insert(0, tr("ALL"));
        defectList.insert(0, tr("ALL"));
        defectList.removeOne(QString("OK"));
        emit openDBDone(levelList, defectList);
    };
    fun();
}

void DefectBrowser::on_ChangeQueryLang()
{
    //get the selected
    QStringList selectLevel = ui->comboBox->getSelected();
    QStringList selectDefect = ui->comboBox_2->getSelected();
    if(selectLevel.size() == 0)
    {
       if(ui->comboBox->currentIndex() >= 0)
       {
           if(ui->comboBox->currentText() != QString("ALL"))
           {
             selectLevel << ui->comboBox->currentText();
           }

       }
    }
    if(selectDefect.size() == 0)
    {
        if(ui->comboBox_2->currentIndex() >= 0)
        {
            if(ui->comboBox_2->currentText() != QString("ALL"))
            {
                selectDefect << ui->comboBox_2->currentText();
            }

        }
    }
    //get search time span
    int start = ui->horizontalSlider->value();
    int end = ui->horizontalSlider_2->value();
    if(end < start)
    {
        return;
    }
    int secsfromBegin = start * 60 * 60;
    int secsfromBegin2 = end * 60 * 60;
    QDateTime setStart = m_startDate.addSecs(secsfromBegin);
    QDateTime setEnd = m_startDate.addSecs(secsfromBegin2);
    int starttm = setStart.toSecsSinceEpoch();
    int endtm = setEnd.toSecsSinceEpoch();
    //generate sqlite command
    QString query1 = QString("select count() from %1").arg(m_book->getShelfName());
    QString query2 = QString("select * from %1").arg(m_book->getShelfName());
     query1 += QString(" where (time between %1 and %2) and w > 1 and h > 1").arg(starttm).arg(endtm);
     query2 += QString(" where (time between %1 and %2) and w > 1 and h > 1").arg(starttm).arg(endtm);
    for (int i = 0; i < selectLevel.size();i++)
    {
        if(i == 0)
        {
            query1 += QString(" and (");
            query2 += QString(" and (");
        }
        query1 += QString(" quality=\'%1\'").arg(selectLevel[i]);
        query2 += QString(" quality=\'%1\'").arg(selectLevel[i]);
        if(i != selectLevel.size() - 1)
        {
            query1 += QString(" or");
            query2 += QString(" or");
        }
        else
        {
            query1 += QString(")");
            query2 += QString(")");
        }
    }

    for(int i = 0; i < selectDefect.size(); i++)
    {
        if(i == 0)
        {
            query1 += QString(" and (");
            query2 += QString(" and (");
        }
        query1 += QString(" type=\'%1\'").arg(selectDefect[i]);
        query2 += QString(" type=\'%1\'").arg(selectDefect[i]);
        if(i != selectDefect.size() - 1)
        {
            query1 += QString(" or");
            query2 += QString(" or");
        }
        else
        {
            query1 += QString(")");
            query2 += QString(")");
        }
    }
    QByteArray qba1 = query1.toUtf8();
    QByteArray qba2 = query2.toUtf8();
    m_book->getBook(qba1.data(), qba1.size(),
                    qba2.data(), qba2.size());
}

void DefectBrowser::on_updateDateTime()

{
    if(!m_startDate.isValid() || !m_startDate.isValid())
    {
        return;
    }
    int start = ui->horizontalSlider->value();
    int end = ui->horizontalSlider_2->value();
    int secsfromBegin = start * 60 * 60;
    int secsfromBegin2 = end * 60 * 60;
    QDateTime setStart = m_startDate.addSecs(secsfromBegin);
    QDateTime setEnd = m_startDate.addSecs(secsfromBegin2);
    ui->label_6->setText(setStart.toString());
    ui->label_7->setText(setEnd.toString());
}

void DefectBrowser::on_OpenDBDone(QStringList levelList, QStringList defectList)
{
    for(int i = 0; i < levelList.size(); i++)
    {
        ui->comboBox->addItem(levelList[i]);
    }
    for(int i = 0; i < defectList.size(); i++)
    {
        ui->comboBox_2->addItem(defectList[i]);
    }
    //默认查找全部
    if(levelList.size() > 0)
    {
        ui->comboBox->setCurrentIndex(0);
    }
    if(defectList.size() > 0)
    {
        ui->comboBox_2->setCurrentIndex(0);
    }

    auto fun = [=]()
    {
        on_ChangeQueryLang();
        changBatchFinish = true;
    };
    fun();
}

DefectBrowser::~DefectBrowser()
{
    delete ui;
    delete m_book;
}


void DefectBrowser::on_pushButton_2_clicked()
{
    this->setVisible(false);
    while(m_loader->exec() != QDialog::Accepted)
    {

    }
    this->setVisible(true);
}
