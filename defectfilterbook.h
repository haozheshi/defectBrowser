#ifndef DEFECTFILTERDAO_H
#define DEFECTFILTERDAO_H

#include <QObject>
#include <QThread>
#include <QList>
#include <QDateTime>
#include "sqlite3.h"
#include "imdb.h"


//This is the source that
enum FlipMode
{
    NextPage,
    PrevPage
};
class queryresult;
class defectFilterBook:public QObject
{
Q_OBJECT
public:
    defectFilterBook();
    // set the library
    bool setSource(const wchar_t* dbpath);
    // get the library
    std::string getSource();
    // get bookshelf list
    QStringList getShelfList();
    // set the bookshelf
    void setShelfName(QString batchName);
    // get the bookshelf
    QString getShelfName();
    // get the catagory list
    QStringList getCatagoryList(QString field);
    // get the catagor count
    QList<int> getLevelAndDefectCount(QList<QStringList>& fields, QString conditions);
    // get the defect historgram
    int getSatisfyCount(const char* query1, int len1);
    // get a new book from the shelf
    void getBook(const char* query1, int len1,
                 const char* query2, int len2);
    // set words number in one page
    void setOnePageCount(int nNumRecord);
    // get the page count
    int getPageNum();
    // get the current page index;
    int getCurrentPage() const;
    // fip book
    bool flipBook(FlipMode);
    // get words from page
    QList<queryresult> getPageWords();
    // get first page time
    QDateTime getFirstPageDate();
    //get list page time
    QDateTime getLastPageDate();
signals:
    // notify page model word updates
    void contentsChange();
private:
    int m_nCurrentPage = -1;
    int m_nTotalPage = -1;
    int m_nPerPageCount = 52;
    sqlite3* m_db;
    ImageDB* m_imdb;
    QString m_queryLang;
    QString m_currentShelf;
    std::string m_strSource;
};

#endif // DEFECTFILTERDAO_H
