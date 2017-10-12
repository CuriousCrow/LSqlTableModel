#ifndef LSQLTABLEMODEL_H
#define LSQLTABLEMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlResult>
#include <QSqlField>
#include <QSqlIndex>

QT_BEGIN_NAMESPACE

class QSqlHelper
{
public:
    // SQL keywords
    inline const static QLatin1String as() { return QLatin1String("AS"); }
    inline const static QLatin1String asc() { return QLatin1String("ASC"); }
    inline const static QLatin1String comma() { return QLatin1String(","); }
    inline const static QLatin1String desc() { return QLatin1String("DESC"); }
    inline const static QLatin1String eq() { return QLatin1String("="); }
    // "and" is a C++ keyword
    inline const static QLatin1String et() { return QLatin1String("AND"); }
    inline const static QLatin1String from() { return QLatin1String("FROM"); }
    inline const static QLatin1String leftJoin() { return QLatin1String("LEFT JOIN"); }
    inline const static QLatin1String on() { return QLatin1String("ON"); }
    inline const static QLatin1String orderBy() { return QLatin1String("ORDER BY"); }
    inline const static QLatin1String parenClose() { return QLatin1String(")"); }
    inline const static QLatin1String parenOpen() { return QLatin1String("("); }
    inline const static QLatin1String select() { return QLatin1String("SELECT"); }
    inline const static QLatin1String sp() { return QLatin1String(" "); }
    inline const static QLatin1String where() { return QLatin1String("WHERE"); }
    inline const static QLatin1String is() { return QLatin1String("IS"); }
    inline const static QLatin1String null() { return QLatin1String("NULL"); }

    // Build expressions based on key words
    inline const static QString as(const QString &a, const QString &b) { return b.isEmpty() ? a : concat(concat(a, as()), b); }
    inline const static QString asc(const QString &s) { return concat(s, asc()); }
    inline const static QString comma(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(comma()).append(b); }
    inline const static QString concat(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : QString(a).append(sp()).append(b); }
    inline const static QString desc(const QString &s) { return concat(s, desc()); }
    inline const static QString eq(const QString &a, const QString &b) { return QString(a).append(eq()).append(b); }
    inline const static QString et(const QString &a, const QString &b) { return a.isEmpty() ? b : b.isEmpty() ? a : concat(concat(a, et()), b); }
    inline const static QString from(const QString &s) { return concat(from(), s); }
    inline const static QString leftJoin(const QString &s) { return concat(leftJoin(), s); }
    inline const static QString on(const QString &s) { return concat(on(), s); }
    inline const static QString orderBy(const QString &s) { return s.isEmpty() ? s : concat(orderBy(), s); }
    inline const static QString paren(const QString &s) { return s.isEmpty() ? s : parenOpen() + s + parenClose(); }
    inline const static QString select(const QString &s) { return concat(select(), s); }
    inline const static QString where(const QString &s) { return s.isEmpty() ? s : concat(where(), s); }
    inline const static QString isNull(const QString &s) { return concat(s, concat(is(),null())); }
};

QT_END_NAMESPACE

typedef QSqlHelper Sql;

class LLookupField;

class LSqlRecord : public QSqlRecord
{
public:
  LSqlRecord();
  LSqlRecord(const QSqlRecord& rec);

  enum CacheAction {None, Insert, Update, Delete};

  void setCacheAction(CacheAction action){ _cacheAction = action; }
  CacheAction cacheAction() const { return _cacheAction; }
private:
  CacheAction _cacheAction;
};

class LSqlTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit LSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

  bool setTable(QString tableName);
  QString tableName();
  void setFilter(QString sqlFilter){ _sqlFilter = sqlFilter; }
  QString filter(){ return _sqlFilter; }
  void setSort(int colIndex, Qt::SortOrder sortOrder);
  void setSort(QString colName, Qt::SortOrder sortOrder);  

  void setHeaders(QStringList strList);

  void addLookupField(LSqlTableModel* lookupModel, QString keyField, QString lookupField);

  int fieldIndex(QString fieldName) const;
  bool isDirty(const QModelIndex & index) const;
  bool isDirty() const;
  void setCacheAction(qlonglong recId, LSqlRecord::CacheAction action);

  void setSequenceName(QString name);
  //Populate model with table data
  virtual bool select();
  //Submit one record by row
  bool submitRow(int row);
  //Submit all cached changes to database
  bool submitAll();
  //Revert all cached changes
  bool revertAll();

  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  QVariant data(int row, QString columnName, int role = Qt::DisplayRole);
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  bool setData(int row, QString columnName, QVariant value, int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex & index) const ;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

  QSqlRecord record(int row) const;
  int rowByValue(QString field, QVariant value);
  QSqlRecord* recordById(qlonglong id);
  QSqlRecord patternRecord() { return _patternRec; }

  //TODO: Should be static method
  QVariant execQuery(const QString &sql, QString resColumn);
  //Wrapper for all sql-queries (for debugging)
  bool execQuery(const QString &sql);

  static void enableLogging(bool enable);
signals:
  void beforeInsert(QSqlRecord &rec);
  void beforeUpdate(QSqlRecord &rec);
private:
  QString _tableName;
  QString _sqlFilter;
  QString _orderByClause;
  QString _sequenceName;
  QStringList _headers;

  typedef QHash<qlonglong, LSqlRecord> CacheMap;
  QSqlIndex _primaryIndex;
  QSqlRecord _patternRec;
  QList<LLookupField> _lookupFields;

  bool _modified = false;

  void setCacheAction(LSqlRecord &rec, LSqlRecord::CacheAction action);
  bool submitRecord(LSqlRecord &rec);
  bool reloadRow(int row);
  bool isNull(const QModelIndex &index);
  //Get next sequence value
  qlonglong nextSequenceNumber();
  bool returningInsertMode();
  int _insertedCount = 0;
  bool _autoIncrementID;

  static bool _logging;
protected:
  CacheMap _recMap;
  QList<qlonglong> _recIndex;

  void clearData();
  QSqlRecord primaryValues(QSqlRecord rec) const;
  qlonglong primaryKey(int row, int part = 0);
  QString primaryKeyName(int part = 0);
  int primaryKeyCount();
  QSqlDatabase _db;
  QSqlQuery _query;

  virtual QString selectAllSql();
  virtual bool selectRowInTable(QSqlRecord &values);
  virtual bool updateRowInTable(const QSqlRecord &values);
  virtual bool insertRowInTable(const QSqlRecord &values);
  virtual bool deleteRowInTable(const QSqlRecord &values);
};

class LLookupField : public QSqlField
{
public:
  LSqlTableModel* lookupModel;
  QString lookupField;
  QString keyField;
  QVariant date(int key);
};

#endif // LSQLTABLEMODEL_H
