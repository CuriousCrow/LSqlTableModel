#include "lsqltreemodel.h"
#include <QDebug>


LSqlTreeModel::LSqlTreeModel(QObject *parent, QSqlDatabase db) :
    QAbstractItemModel(parent)
{
  _db = db.isValid() ? db : QSqlDatabase::database();
  _query = QSqlQuery(_db);
}

LSqlTreeModel::~LSqlTreeModel()
{
}

qlonglong LSqlTreeModel::itemParentId(LSqlRecord rec) const
{
  return rec.value(_parentField).toLongLong();
}

QModelIndex LSqlTreeModel::indexById(qlonglong id)
{
  if (!_recMap.contains(id))
    return QModelIndex();
  LSqlRecord itemRec = _recMap[id];
  qlonglong parentId = itemParentId(itemRec);
  int row;
  if (parentId == 0){
    row = _rootRecList.indexOf(id);
  }
  else {
    LSqlRecord parentRec = _recMap[parentId];
    row = parentRec.childIndex(id);
  }
  return createIndex(row, 0, id);
}

void LSqlTreeModel::clear()
{
  _recMap.clear();
  _rootRecList.clear();
}

void LSqlTreeModel::addRecord(LSqlRecord rec)
{
  qlonglong parentId = itemParentId(rec);

  if (parentId == 0){
    _rootRecList.append(rec.id());
  } else if (!_recMap.contains(parentId)){
    //Указанный родитель не найден
    return;
  } else {
    _recMap[parentId].addChild(rec.id());
  }
  QModelIndex parentIdx = indexById(parentId);
  beginInsertRows(parentIdx, rowCount(parentIdx) - 1, rowCount(parentIdx) - 1);
  _recMap.insert(rec.id(), rec);
  endInsertRows();
}

bool LSqlTreeModel::setTable(QString tableName)
{
  _tableName = tableName;
  _patternRec = _db.record(_tableName);
  _primaryIndex = _db.primaryIndex(_tableName);
  return true;
}

bool LSqlTreeModel::select()
{
  QString selectAllSql = "select * from %1";
  if (!execQuery(selectAllSql.arg(_tableName))){
    return false;
  }
  beginResetModel();
  clear();

  while (_query.next()){
    qlonglong parentId = _query.value(_parentField).toLongLong();
    qlonglong id = _query.value(PRIMARY_KEY_FIELD).toLongLong();
    if (_recMap.contains(id)){
      _recMap[id].copyValues(_query.record());
    }
    else {
      _recMap.insert(id, LSqlRecord(_query.record()));
    }
    if (parentId == 0){
      _rootRecList.append(id);
    }
    else {
      if (!_recMap.contains(parentId)){
        _recMap.insert(parentId, LSqlRecord(_patternRec));
      }
      _recMap[parentId].addChild(id);
    }
  }
  endResetModel();

  return true;
}

QModelIndex LSqlTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();    

    qlonglong childId;

    //Если родительский индекс невалидный, значит это корневой элемент
    if (!parent.isValid()){
      childId = _rootRecList.at(row);
    }
    //получаем элемент родителя и по номеру элемента получает ID
    else {
      LSqlRecord parentRec = _recMap[parent.internalId()];
      childId = parentRec.childByIndex(row);
    }        
    if (childId > 0)
      return createIndex(row, column, childId);
    else
      return QModelIndex();
}

QModelIndex LSqlTreeModel::parent(const QModelIndex &child) const
{
  if (!child.isValid())
    return QModelIndex();

  //получаем ID родительского элемента
  LSqlRecord childItem = _recMap[child.internalId()];
  qlonglong parentId = itemParentId(childItem);
  //если PARENT_ID не заполнен, значит родительского элемента нет
  if (parentId == 0){
    return QModelIndex();
  }
  else {
    //получаем ID дедушки
    LSqlRecord parentRec = _recMap[parentId];
    qlonglong grandParentId = itemParentId(parentRec);
    int parentRowIndex;
    //если не удалось получить ID дедушки, значит родитель - корневой элемент
    if (grandParentId == 0){
      parentRowIndex = _rootRecList.indexOf(parentId);
    }
    else {      
      //находим номер родителя в списке дедушкиных детей
      LSqlRecord grandParentItem = _recMap[grandParentId];
      parentRowIndex = grandParentItem.childIndex(parentId);
    }
    return createIndex(parentRowIndex, 0, parentId);
  }
}

int LSqlTreeModel::rowCount(const QModelIndex &parent) const
{
  //дочерние элементы есть только у первой колонки
  if (parent.column() > 0)
    return 0;

  //если родительский индекс невалидный, возвращаем кол-во корневых элементов
  if (!parent.isValid()){
    return _rootRecList.count();
  }
  else {
    //возвращаем кол-во дочерних элементов
    LSqlRecord parentRec = _recMap[parent.internalId()];
    return parentRec.childCount();
  }
}

int LSqlTreeModel::columnCount(const QModelIndex &parent) const
{
  //кол-во колонок таблицы
  return _patternRec.count();
}

QVariant LSqlTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  //получаем запись таблицы по ID
  LSqlRecord rec = _recMap[index.internalId()];
  int column = index.column();

  switch (role) {
  case Qt::DisplayRole:    
    return rec.value(column);
    break;
  default:
    return QVariant();
    break;
  }
}

void LSqlTreeModel::setParentField(QString name)
{
  _parentField = name;
}

LSqlRecord LSqlTreeModel::patternRec() const
{
  return _patternRec;
}

int LSqlTreeModel::fieldIndex(QString fieldName) const
{
  return _patternRec.indexOf(fieldName);
}

LSqlRecord LSqlTreeModel::recById(qlonglong id) const
{
  return _recMap[id];
}

bool LSqlTreeModel::execQuery(const QString &sql)
{
  bool result = _query.exec(sql);
  qDebug() << "Execute query: " << sql;
  if (!result){
    qDebug() << "Error: " << _query.lastError().databaseText();
  }
  return result;
}

QVariant LSqlTreeModel::execQuery(const QString &sql, QString resColumn)
{
  if (execQuery(sql) && _query.next()){
    return _query.value(resColumn);
  }
  else {
    return QVariant();
  }
}

LSqlRecord::LSqlRecord(): QSqlRecord()
{
}

LSqlRecord::LSqlRecord(const QSqlRecord &rec): QSqlRecord(rec)
{
}

LSqlRecord::~LSqlRecord()
{
}

void LSqlRecord::copyValues(const QSqlRecord &rec)
{
  for (int i=0; i<rec.count(); i++){
    setValue(i, rec.value(i));
  }
}

void LSqlRecord::addChild(long childId)
{
  _childIdList.append(childId);
}

void LSqlRecord::removeChild(long childId)
{
  _childIdList.removeOne(childId);
}

int LSqlRecord::childIndex(qlonglong childId)
{
  return _childIdList.indexOf(childId);
}

qlonglong LSqlRecord::childByIndex(int index)
{
  return _childIdList.value(index);
}

int LSqlRecord::childCount()
{
  return _childIdList.count();
}


QVariant LSqlTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
    return _patternRec.fieldName(section);
  default:
    return QVariant();
  }
}
