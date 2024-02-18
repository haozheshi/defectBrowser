#include "defectpagepage.h"
#include "queryresult.h"

defectPage::defectPage(defectFilterBook* book, QObject* parent)
    :QAbstractListModel(parent)
{
    m_book = book;
}

defectPage::~defectPage()
{

}

int defectPage::rowCount(const QModelIndex& /*parent*/) const
{
    return  m_words.count();
}

QVariant defectPage::data(const QModelIndex& index, int role) const
{
    if (!isIndexValid(index))
    {
           return QVariant();
    }
    int row = index.row();
    switch (role)
    {
        case Qt::DisplayRole:
            return m_words[row].m_strTitle;
        case Roles::rectRole:
            return m_words[row].m_rt;
        case Roles::countRole:
            return m_words[row].m_strCount;
        case Roles::majorRole:
            return m_words[row].m_bMajor;
        case Roles::fileRole:
            return m_words[row].m_strArchive;
        case Roles::timeRole:
            return m_words[row].m_strTime;
        case Roles::remarkRole:
            return m_words[row].m_strRemark;
        case Roles::imageRole:
            return m_words[row].m_imPhoto;
        default:
            return QVariant();
    }
}

bool defectPage::isIndexValid(const QModelIndex& index) const
{
    if (index.row() < 0
            || index.row() >= rowCount()
            || !index.isValid()) {
        return false;
    }
    return true;
}

void defectPage::updateWords()
{
    if(m_book != nullptr)
    {
        beginResetModel();
        m_words = m_book->getPageWords();
        endResetModel();
    }
}
