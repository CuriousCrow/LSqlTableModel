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
};

QT_END_NAMESPACE

typedef QSqlHelper Sql;

class LSqlRecord : public QSqlRecord
{
public:
  LSqlRecord();
  LSqlRecord(const QSqlRecord& rec);

  enum CacheAction {None, Insert, Update, Delete};

  void setCacheAction(CacheAction action){ _cacheAction = action; }
  CacheAction cacheAction(){ return _cacheAction; }
private:
  CacheAction _cacheAction;
};

class LSqlTableModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  explicit LSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());

  bool setTable(QString tableName);
  void setFilter(QString sqlFilter){ _sqlFilter = sqlFilter; }
  QString tableName();
  int fieldIndex(QString fieldName);
  void setSequenceName(QString name){ _sequenceName = name; }
  //Загрузка данных таблицы в модель
  bool select();
  //Сохранение всех закэшированных изменений
  bool submitAll();
  //Откат всех закэшированных изменений
  bool revertAll();

  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex & index) const ;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

  bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
  bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

  QSqlRecord record(int row) const;
signals:
  void beforeInsert(QSqlRecord &rec);
  void beforeUpdate(QSqlRecord &rec);
public slots:

private:
  QSqlDatabase _db;
  QSqlQuery _query;

  QString _tableName;
  QString _sqlFilter;
  QString _sequenceName;

  typedef QHash<long, LSqlRecord> CacheMap;
  CacheMap _recMap;
  QList<long> _recIndex;

  QSqlRecord _patternRec;
  QSqlIndex _primaryIndex;
  bool _modified = false;

  void setCacheAction(LSqlRecord &rec, LSqlRecord::CacheAction action);
  bool submitRecord(LSqlRecord &rec);
  bool reloadRow(int row);
  bool selectRowInTable(QSqlRecord &values);
  bool updateRowInTable(const QSqlRecord &values);
  bool insertRowInTable(const QSqlRecord &values);
  bool deleteRowInTable(const QSqlRecord &values);
  void clearData();
  QSqlRecord primaryValues(QSqlRecord rec) const;
  //Получение следующего значения генератора
  int nextId();
  //Обертка для sql-запрос для отладки
  bool execQuery(const QString &sql);
};

#endif // LSQLTABLEMODEL_H
