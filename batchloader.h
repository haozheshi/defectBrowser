#ifndef BATCHLOADER_H
#define BATCHLOADER_H

#include <QDialog>
#include <QStringListModel>

namespace Ui {
class batchloader;
}

class batchloader : public QDialog
{
    Q_OBJECT
public:
    explicit batchloader(QWidget *parent = nullptr);
    ~batchloader();
signals:
    void openBatch(QString source);
private slots:
    void on_openBatch_clicked(const QModelIndex &index);
    void on_buttonBox_accepted();

public:
    QString m_strBatchPath;
    QString m_strRoot;
private:
    Ui::batchloader *ui;
    QStringListModel model;
    QStringList m_strBatchDirList;
};

#endif // BATCHLOADER_H
