#include "defectfilterbook.h"
#include "queryresult.h"
#include <codecvt>
#include <QString>
#include <QElapsedTimer>
#include <QDebug>

defectFilterBook::defectFilterBook()
{
    m_db = nullptr;
    m_imdb = new ImageDB();
    m_queryLang.clear();
}

bool defectFilterBook::setSource(const wchar_t * dbpath_)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    if(m_db != nullptr)
    {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
    std::wstring wdbpath = std::wstring(dbpath_);

    m_imdb->set();

    if (m_imdb != nullptr)
    {
        int npos = wdbpath.rfind(L'/');
        std::wstring wdbhome = wdbpath.substr(0, npos);
        std::string dbhome = converter.to_bytes(wdbhome);
        std::wstring dbname = wdbpath.substr(npos+1);
        dbname = dbname.substr(0, dbname.length() - 3);
        m_imdb->open(dbhome.c_str(), dbname.c_str());
        m_imdb->setReadOnly();
    }

    m_strSource = converter.to_bytes(wdbpath);
    int res = sqlite3_open(m_strSource.c_str(), &m_db);
    if (res == SQLITE_OK)
    {
        sqlite3_exec(m_db,"create index search_index on defectTable(time,quality,judge,type)", 0, nullptr, nullptr);
        return true;
    }
    else
    {
        m_strSource.clear();
        m_db = nullptr;
         qDebug() << "open failed"<< QString::fromStdWString(dbpath_);
        return false;
    }

}

std::string defectFilterBook::getSource()
{
    return m_strSource;
}


QStringList defectFilterBook::getShelfList()
{
    QStringList shelfList;
    if(m_db != nullptr)
    {
        sqlite3_stmt* stmt = nullptr;
        std::string sqlcom = "select name from sqlite_master where type='table';";
        sqlite3_prepare(m_db, sqlcom.c_str(), int(sqlcom.length()), &stmt, NULL);
        int ret = SQLITE_FAIL;
        if(stmt != nullptr)
        {
           ret = sqlite3_step(stmt);
        }
        while (ret == SQLITE_ROW)
        {
            int nBytes = sqlite3_column_bytes(stmt, 0);
            const unsigned char* tableName = sqlite3_column_text(stmt, 0);
            QString shelfName;
            shelfName = shelfName.fromUtf8((char*)tableName, nBytes);
            if(shelfName != QString("sqlite_sequence"))
            {
                shelfList << shelfName;
            }
            ret = sqlite3_step(stmt);
            if (ret == SQLITE_DONE)
            {
                sqlite3_finalize(stmt);
                return shelfList;
            }
        }
        sqlite3_finalize(stmt);
    }
    return shelfList;
}

void defectFilterBook::setShelfName(QString shelfName)
{
    m_currentShelf = shelfName;
}

QString defectFilterBook::getShelfName()
{
    return m_currentShelf;
}

QStringList defectFilterBook::getCatagoryList(QString field)
{
    QStringList catagoryList;
    if(m_db != nullptr)
    {
        if(!m_currentShelf.isEmpty())
        {
            sqlite3_stmt* stmt = nullptr;
            QString wsqlcom = QString("select distinct %1  from \'%2\'").arg(field).arg(m_currentShelf);
            QByteArray qba = wsqlcom.toUtf8();
            sqlite3_prepare(m_db, qba.data(), qba.size(), &stmt, NULL);
            int ret = SQLITE_FAIL;
            if(stmt != nullptr)
            {
                ret = sqlite3_step(stmt);
            }
            while (ret == SQLITE_ROW)
            {
                int nBytes = sqlite3_column_bytes(stmt, 0);
                const unsigned char* defect = sqlite3_column_text(stmt, 0);
                QString catagoryName;
                catagoryName = catagoryName.fromUtf8((char*)defect, nBytes);
                catagoryList << catagoryName;
                ret = sqlite3_step(stmt);
                if (ret == SQLITE_DONE)
                {
                    sqlite3_finalize(stmt);
                    return catagoryList;
                }
            }
            sqlite3_finalize(stmt);
        }
    }
    return catagoryList;
}


 QList<int> defectFilterBook::getLevelAndDefectCount(QList<QStringList>& fields, QString conditions)
 {
     QList<int> res;
     if(m_db != nullptr)
     {
        sqlite3_stmt* stmt = nullptr;

        QString query = "select quality,type,count() from defectTable";
        if(!conditions.isEmpty())
        {
            query += QString(" where %1").arg(conditions);
        }
        query += QString(" group by quality,type");
        QByteArray qba = query.toUtf8();
        sqlite3_prepare(m_db, qba.data(), qba.size(),&stmt, nullptr);
        if(stmt != nullptr)
        {
            int ret = sqlite3_step(stmt);
            while (ret == SQLITE_ROW)
            {
                int ncolCount = sqlite3_column_count(stmt);
                if(ncolCount > 0)
                {
                    QStringList field;
                    QString level;
                    const unsigned char* text = sqlite3_column_text(stmt, 0);
                    level = level.fromUtf8((const char*)text);
                    QString defect;
                    text = sqlite3_column_text(stmt, 1);
                    defect = defect.fromUtf8((const char*)text);
                    int ncount = sqlite3_column_int(stmt, 2);
                    field << level;
                    field << defect;
                    fields << field;
                    res << ncount;
                }
                else
                {
                    sqlite3_finalize(stmt);
                    return res;
                }
                ret = sqlite3_step(stmt);
                if (ret == SQLITE_DONE)
                {
                    sqlite3_finalize(stmt);
                    return res;
                }
            }
            sqlite3_finalize(stmt);
        }
     }
     return res;
 }



int defectFilterBook::getSatisfyCount(const char* query1, int len1)
{
    int nTotal = 0;
    //执行查询语句
//    qDebug() << QString::fromUtf8(query1, len1);
    if(m_db != nullptr)
    {
       sqlite3_stmt* stmt = nullptr;
       sqlite3_prepare(m_db, query1, len1, &stmt, nullptr);
//       qDebug() << QString::fromUtf8(query1, len1) << "prepare done";
       if(stmt != nullptr)
       {
           int ret = sqlite3_step(stmt);
           if(ret == SQLITE_ROW)
           {
               int ncolCount = sqlite3_column_count(stmt);
               if(ncolCount > 0)
               {
                    nTotal = sqlite3_column_int(stmt, 0);
               }
               else
               {
                    nTotal = 0;
               }

           }
           sqlite3_finalize(stmt);
       }
    }
    return nTotal;
}

void defectFilterBook::getBook(const char* query1, int len1,
        const char* query2, int len2)
{
    Q_UNUSED(query1)
    Q_UNUSED(len1)
    m_queryLang.clear();
    //获取
    m_queryLang = m_queryLang.fromUtf8(query2, len2);
    m_nTotalPage = 1;
    m_nCurrentPage = 1;
    emit contentsChange();
}


bool defectFilterBook::flipBook(FlipMode mode)
{
    if(mode == NextPage)
    {
        if(m_nCurrentPage < m_nTotalPage)
        {
            m_nCurrentPage += 1;
            emit contentsChange();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(m_nCurrentPage > 1)
        {
            m_nCurrentPage -=1;
            emit contentsChange();
            return true;
        }
        else
        {
            return false;
        }
    }
}

void defectFilterBook::setOnePageCount(int nNumRecord)
{
    m_nPerPageCount = nNumRecord;
}


// table info
//|id(缺陷ID)|count(计数)|time(检测时间)|quality(产品等级)|judge(是否为判定缺陷)|type(缺陷类型)|archive(原图路径)|
//|x(缺陷坐标)|y(缺陷坐标)|w(缺陷坐标)|h(缺陷坐标)|snapshot(缺陷小图)|remark(备注)|
QList<queryresult> defectFilterBook::getPageWords()
{
    QElapsedTimer time;
    QList<queryresult> res;
    if(m_db != nullptr)
    {
        time.start();
        int nOffset = (m_nCurrentPage - 1) * m_nPerPageCount;
        QString limitQuey = QString("%1 ORDER BY id limit %2 offset %3;").arg(m_queryLang).arg(m_nPerPageCount).arg(nOffset);
        sqlite3_stmt* stmt = nullptr;
        QByteArray qba = limitQuey.toUtf8();
        sqlite3_prepare(m_db, qba.data(), qba.size(), &stmt, nullptr);
        if(stmt != nullptr)
        {
            int ret = sqlite3_step(stmt);
            while(ret == SQLITE_ROW)
            {
                 int id = sqlite3_column_int(stmt, 0);
                 int count = sqlite3_column_int(stmt, 1);
                 int nByte;
                 time_t ts = sqlite3_column_int(stmt, 2);
                 struct tm ptm_out;
                 localtime_s(&ptm_out, &ts);
                 QString inpectTime = QString("%1-%2-%3 %4:%5:%6").arg(ptm_out.tm_year + 1900).arg(ptm_out.tm_mon + 1).arg(ptm_out.tm_mday).arg(ptm_out.tm_hour).arg(ptm_out.tm_min).arg(ptm_out.tm_sec);
                 const unsigned char* level = sqlite3_column_text(stmt, 3);
                 nByte = sqlite3_column_bytes(stmt, 3);
                 QString strLevel = QString::fromUtf8((const char*)level, nByte);
                 bool bmajor = sqlite3_column_int(stmt, 4);
                 const unsigned char* type = sqlite3_column_text(stmt, 5);
                 nByte = sqlite3_column_bytes(stmt, 5);
                 QString strDefect = QString::fromUtf8((const char*)type, nByte);
                 const unsigned char* archive = sqlite3_column_text(stmt, 6);
                 nByte = sqlite3_column_bytes(stmt, 6);
                 QString strAchive = QString::fromUtf8((const char*)archive, nByte);
                 int x = sqlite3_column_int(stmt, 7);
                 int y = sqlite3_column_int(stmt, 8);
                 int w = sqlite3_column_int(stmt, 9);
                 int h = sqlite3_column_int(stmt, 10);
                 int nSub = sqlite3_column_int(stmt, 11);
                 queryresult qr;
                 if(m_imdb != nullptr)
                 {
                     std::shared_ptr<char> im;
                     int nsize;
                     m_imdb->getImage(nSub, id, im, nsize);
                     QByteArray qba((const char*)im.get(), nsize);
                     qr.m_imPhoto = qba;
                 }
                 else
                 {
                    qr.m_imPhoto = QByteArray();
                 }

                 const unsigned char* remark = sqlite3_column_text(stmt, 12);
                 nByte = sqlite3_column_bytes(stmt, 12);

                 qr.m_strCount = QString("%1").arg(count);
                 qr.m_strTitle = QString("%1-%2").arg(strLevel).arg(strDefect);

                 qr.m_strTime = inpectTime;
                 qr.m_strRemark = QString::fromUtf8((const char*)remark,nByte);
                 qr.m_strArchive = strAchive;
                 qr.m_rt = QRect(x, y, w, h);
                 qr.m_bMajor =bmajor;
                 res << qr;
                 ret = sqlite3_step(stmt);
            }
            sqlite3_finalize(stmt);
        }
        if(res.size() == m_nPerPageCount)
        {
            m_nTotalPage= m_nCurrentPage + 1;
        }
    }
    qDebug() << "The search took" << time.elapsed() << "milliseconds";
    return res;
}

QDateTime defectFilterBook::getFirstPageDate()
{
    if(m_db != nullptr)
    {
        QString limitQuey = QString("select * from \'%1\' order by id limit 1 ").arg(m_currentShelf);
        sqlite3_stmt* stmt = nullptr;
        QByteArray qba = limitQuey.toUtf8();
        sqlite3_prepare(m_db, qba.data(), qba.size(), &stmt, nullptr);
        if(stmt != nullptr)
        {
            int ret = sqlite3_step(stmt);
            while(ret == SQLITE_ROW)
            {
                time_t ts = sqlite3_column_int(stmt, 2);
                QDateTime qdt = QDateTime::fromSecsSinceEpoch(ts);
                sqlite3_finalize(stmt);
                return qdt;
            }
            sqlite3_finalize(stmt);
        }
    }
    return QDateTime();
}

QDateTime defectFilterBook::getLastPageDate()
{
    if(m_db != nullptr)
    {
        QString limitQuey = QString("select * from \'%1\' order by id desc limit 1 ").arg(m_currentShelf);
        sqlite3_stmt* stmt = nullptr;
        QByteArray qba = limitQuey.toUtf8();
        sqlite3_prepare(m_db, qba.data(), qba.size(), &stmt, nullptr);
        if(stmt != nullptr)
        {
            int ret = sqlite3_step(stmt);
            while(ret == SQLITE_ROW)
            {
                time_t ts = sqlite3_column_int(stmt, 2);
                QDateTime qdt = QDateTime::fromSecsSinceEpoch(ts);
                sqlite3_finalize(stmt);
                return qdt;
            }
            sqlite3_finalize(stmt);
        }
    }
    return QDateTime();
}
