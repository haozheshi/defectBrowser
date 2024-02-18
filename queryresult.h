#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include <QString>
#include <QByteArray>
#include <QRect>

class queryresult
{
public:
    queryresult();
public:
    QString m_strCount;
    QString m_strTitle;
    QByteArray m_imPhoto;
    QString m_strTime;
    QString m_strRemark;
    QString m_strArchive;
    QRect m_rt;
    bool m_bMajor;
};

#endif // QUERYRESULT_H
