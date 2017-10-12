#ifndef LSQLTREEMODEL_H
#define LSQLTREEMODEL_H

#include <QObject>
#include <QAbstractItemModel>
#include <QSqlDriver>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlError>

#define PRIMARY_KEY_FIELD "ID"
#define PARENT_ID_FIELD "PARENT_ID"

class LSqlRecord : public QSqlRecord
{
public:
  LSqlRecord();
  LSqlRecord(const QSqlRecord& rec);
  ~LSqlRecord();
  void copyValues(const QSqlRecord& rec);
  void addChild(long childId);
  void removeChild(long childId);
  qlonglong childByIndex(int index);
  int childIndex(qlonglong childId);
  int childCount();
  qlonglong id(){ return value(PRIMARY_KEY_FIELD).toLongLong(); }
private:
  QList<qlonglong> _childIdList;
};

class LSqlTreeModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit LSqlTreeModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
  ~LSqlTreeModel();
private:
  QSqlDatabase _db;
  QSqlQuery _query;
  QMap<qlonglong, LSqlRecord> _recMap;
  QList<qlonglong> _rootRecList;
  QString _tableName;
  QSqlIndex _primaryIndex;
  QSqlRecord _patternRec;
  QString _parentField = PARENT_ID_FIELD;
  qlonglong itemParentId(LSqlRecord rec) const;
  QModelIndex indexById(qlonglong id);
  void clear();
public:
  void addRecord(LSqlRecord rec);
  bool setTable(QString tableName);  
  virtual bool select();
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &child) const;
  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  void setParentField(QString name);
  LSqlRecord patternRec() const;
  int fieldIndex(QString fieldName) const;
  LSqlRecord recById(qlonglong id) const;
  //TODO: Should be static method
  QVariant execQuery(const QString &sql, QString resColumn);
  //Wrapper for all sql-queries (for debugging)
  bool execQuery(const QString &sql);    
};

#endif // LSQLTREEMODEL_H
