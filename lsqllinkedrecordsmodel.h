#ifndef LSQLLINKEDRECORDSMODEL_H
#define LSQLLINKEDRECORDSMODEL_H

#include "lsqltablemodel.h"

class LSqlLinkedRecordsModel : public LSqlTableModel
{
  Q_OBJECT
public:
  explicit LSqlLinkedRecordsModel(QObject *parent = 0);
  void setLinkField(QString name){ _linkField = name; }
  void setInitialLinkValue(QVariant value){ _initialLinkValue = value; }

protected:
  virtual QString selectAllSql();
private:
  QString _linkField;
  QVariant _initialLinkValue = 0;
public:
  virtual void sort(int column, Qt::SortOrder order);
  virtual bool insertRows(int row, int count, const QModelIndex &parent);
  virtual bool removeRows(int row, int count, const QModelIndex &parent);
  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
};

#endif // LSQLLINKEDRECORDSMODEL_H
