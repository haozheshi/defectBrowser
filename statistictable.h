#ifndef STATISTICTABLE_H
#define STATISTICTABLE_H

#include <QWidget>
#include <QAbstractItemModel>
#include <QTreeView>

struct statisticData
{
    QString defect;
    int count;
    int nLevel;
};

class statisticModel:public QAbstractItemModel
{
    Q_OBJECT
   public:
       explicit statisticModel(QObject *parent = nullptr);
        ~statisticModel(){};

       QVariant data(const QModelIndex &index, int role) const override;
       Qt::ItemFlags flags(const QModelIndex &index) const override;
       QVariant headerData(int section, Qt::Orientation orientation,
                           int role = Qt::DisplayRole) const override;
       QModelIndex index(int row, int column,
                         const QModelIndex &parent = QModelIndex()) const override;
       QModelIndex parent(const QModelIndex &index) const override;
       int rowCount(const QModelIndex &parent = QModelIndex()) const override;
       int columnCount(const QModelIndex &parent = QModelIndex()) const override;
       void setModelData(QList<QString> levelName, QList<QList<statisticData>> data);
       QList<QList<statisticData>> m_data;
       QList<QString> m_levelName;
       int totalcount = 0;
};

class statisticTable: public QWidget
{
Q_OBJECT
public:
    explicit statisticTable(QWidget *parent = nullptr);
    ~statisticTable();
    void setModelData(QList<QString> levelName, QList<QList<statisticData>> data);
private:
    statisticModel m_statistic;
public:
    QTreeView* m_treeView;
};

#endif // STATISTICTABLE_H
