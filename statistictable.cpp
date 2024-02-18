#include "statistictable.h"
#include <QVBoxLayout>
#include <QAbstractItemModel>
#include <QDebug>


statisticModel::statisticModel(QObject *parent):
    QAbstractItemModel(parent)
{

}

void statisticModel::setModelData(QList<QString> levelName,
                                  QList<QList<statisticData>> data)
{
    beginResetModel();
    m_levelName.clear();
    m_data.clear();
    m_levelName = levelName;
    m_data = data;
    totalcount = 0;
    for(auto& l: data)
    {
        for(auto& d: l)
        {
            totalcount += d.count;
        }
    }
    endResetModel();
}


QModelIndex statisticModel::index(int row, int column, const QModelIndex &parent) const
{
    if(!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }
    if(!parent.isValid())
    {
        if(row >= m_data.size() || column >= 3)
        {
            return QModelIndex();
        }
        else
        {
            return createIndex(row, column, nullptr);
        }
    }
    else
    {
        if(parent.internalPointer() == nullptr)
        {
            int nrow = parent.row();
            const statisticData* dat =  &m_data[nrow][row];
            return createIndex(row, column, (void*)dat);
        }
        else
        {
            return QModelIndex();
        }

    }
}

QModelIndex statisticModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
        return QModelIndex();
    if(index.internalPointer() == nullptr)
    {
        return  QModelIndex();
    }
    else
    {
        statisticData* defect = static_cast<statisticData*>(index.internalPointer());
        return createIndex(defect->nLevel, 0, nullptr);
    }

}


int statisticModel::rowCount(const QModelIndex &parent) const
{
    if(parent.column() > 0)
    {
        return 0;
    }
    if(!parent.isValid())
    {
        return m_data.size();
    }
    else
    {
        if(parent.internalPointer() == nullptr)
        {
            int nrow = parent.row();
            return m_data[nrow].size();
        }
        else
        {
            return 0;
        }
    }
}



int statisticModel::columnCount(const QModelIndex &parent) const
{
   Q_UNUSED(parent)
   return 3;
}

Qt::ItemFlags statisticModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}


QVariant statisticModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    statisticData* defect = static_cast<statisticData*>(index.internalPointer());
    if(defect == nullptr)
    {
        int nrow = index.row();
        if(index.column() == 0)
        {
            return m_levelName[nrow];
        }
        else if(index.column() == 1)
        {
            int levelcount = 0;
            for(int i = 0; i < m_data[nrow].size(); i++)
            {
                levelcount += m_data[nrow][i].count;
            }
            return QVariant(levelcount);
        }
        else if(index.column() == 2)
        {
            if(totalcount == 0)
            {
                return QVariant(0.0);
            }
            else
            {
                int levelcount = 0;
                for(int i = 0; i < m_data[nrow].size(); i++)
                {
                    levelcount += m_data[nrow][i].count;
                }
                return QVariant(100.0 * levelcount / totalcount);
            }
        }
        else
        {
            return QVariant();
        }
    }
    else
    {
        if(index.column() == 0)
        {
            return defect->defect;
        }
        else if(index.column() == 1)
        {

            return defect->count;
        }
        else if(index.column() == 2)
        {
            if(totalcount == 0)
            {
                return QVariant(0.0);
            }
            else
            {
                return QVariant(100.0 * defect->count / totalcount);
            }
        }
        else
        {
            return QVariant();
        }
    }
}

QVariant statisticModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
       if(section == 0)
       {
            return QString(tr("Type"));
       }
       else if(section == 1)
       {
            return QString(tr("Count"));
       }
       else if(section == 2)
       {
            return QVariant(tr("Percent"));
       }
       else
       {
            return QVariant();
       }
    }
    return QVariant();
}


statisticTable::statisticTable(QWidget *parent):
    QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    this->setLayout(layout);
    m_treeView = new QTreeView(this);
    m_treeView->setModel(&m_statistic);
    m_treeView->setColumnWidth(0, 200);
    layout->addWidget(m_treeView);
}

void statisticTable::setModelData(QList<QString> levelName, QList<QList<statisticData> > data)
{
    m_statistic.setModelData(levelName, data);
}

statisticTable::~statisticTable()
{

}
