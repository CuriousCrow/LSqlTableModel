#include "lsqllinkedrecordsmodel.h"
#include <QDebug>

LSqlLinkedRecordsModel::LSqlLinkedRecordsModel(QObject *parent) :
  LSqlTableModel(parent)
{
}

void LSqlLinkedRecordsModel::sort(int column, Qt::SortOrder order)
{
  //Sorting makes no sense with linked records table
}


QString LSqlLinkedRecordsModel::selectAllSql()
{
  if (_primaryIndex.count() == 1 && _patternRec.contains(_linkField)){
    QString sqlTemplate = "WITH RECURSIVE Rec AS (%1 where %2 = 0 %3\nUNION ALL\n%1, Rec WHERE %2 = Rec.%4)\nSELECT * FROM Rec";
    QString sqlNormalSelect =
      _db.driver()->sqlStatement(QSqlDriver::SelectStatement, _tableName, _patternRec, false);
    QString where = _sqlFilter.isEmpty() ? "" : "and " + _sqlFilter;
    return sqlTemplate.arg(sqlNormalSelect, _linkField, where, _primaryIndex.fieldName(0));
  }
  else {
    qDebug() << "Primary key must be simple and proper linkField should be set";
    return LSqlTableModel::selectAllSql();
  }
}


bool LSqlLinkedRecordsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  if (row < 0 || row > rowCount())
    return false;
  int prevRecID;
  if (row == 0){
    prevRecID = data(row, _linkField).toInt();
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
    int insertedID = primaryKey(row);
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
  int prevId = data(row, _linkField).toInt();
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
