#include "lsqllinkedrecordsmodel.h"

LSqlLinkedRecordsModel::LSqlLinkedRecordsModel(QObject *parent, QSqlDatabase db) :
  LSqlTableModel(parent, db)
{
}

bool LSqlLinkedRecordsModel::select()
{
  if (!execQuery(selectAllSql())){
    return false;
  }
  beginResetModel();
  clearData();
  QMap<int, QSqlRecord> tempMap = QMap<int, QSqlRecord>();
  //filling of record map
  while (_query.next()){
    tempMap.insert(_query.value(_linkField).toInt(), _query.record());
  }
  int parentId = _initialLinkValue.toInt();
  while (true){
    if (!tempMap.contains(parentId))
      break;
    QSqlRecord rec = tempMap.value(parentId);
    _recIndex.append(rec.value("ID").toInt());
    _recMap.insert(rec.value("ID").toInt(), LSqlRecord(rec));
    parentId = rec.value("ID").toInt();
  }
  endResetModel();

  return true;
}

void LSqlLinkedRecordsModel::sort(int column, Qt::SortOrder order)
{
  //sorting makes no sense in a such model type

  //supressing warnings
  std::ignore = column;
  std::ignore = order;
}

bool LSqlLinkedRecordsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (row < 0 || row > rowCount())
    return false;
  qlonglong prevRecID;
  if (row == 0){
    prevRecID = data(row, _linkField).toLongLong();
  }
  else {
    prevRecID = primaryKey(row - 1);
  }
  LSqlTableModel::insertRows(row, count, parent);
  setData(row, _linkField, prevRecID);
  bool result = submitRow(row);
  if (!result)
    removeRow(row);

  if (result && (rowCount() > row + 1)){
    qlonglong insertedID = primaryKey(row);
    setData(row + 1, _linkField, insertedID);
    result = result && submitRow(row + 1);
  }

  return result;
}

bool LSqlLinkedRecordsModel::removeRows(int row, int count, const QModelIndex &parent)
{
  if (row < 0 || row >= rowCount())
    return false;

  //Get ID of previous record from linkField
  qlonglong prevId = data(row, _linkField).toLongLong();
  if (!LSqlTableModel::removeRows(row, count, parent)){
    return false;
  }
  if (rowCount() > row){
    //Update and submit linkField of next record
    return setData(row, _linkField, prevId) && submitRow(row);
  }
  return true;
}

Qt::ItemFlags LSqlLinkedRecordsModel::flags(const QModelIndex &index) const
{
  if (index.column() == fieldIndex(_linkField)){
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }
  return LSqlTableModel::flags(index);
}
