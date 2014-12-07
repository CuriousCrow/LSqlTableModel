#include "lsqllinkedrecordsmodel.h"
#include <QDebug>

LSqlLinkedRecordsModel::LSqlLinkedRecordsModel(QObject *parent, QSqlDatabase db) :
  LSqlTableModel(parent, db)
{
}

void LSqlLinkedRecordsModel::sort(int column, Qt::SortOrder order)
{
  //Sorting makes no sense with linked records table
}


QString LSqlLinkedRecordsModel::selectAllSql()
{
  if (primaryKeyCount() == 1 && fieldIndex(_linkField) >= 0){
    QString sqlTemplate = "WITH RECURSIVE Rec AS (%1 where %2 %3\nUNION ALL\n%1, Rec WHERE %4 = Rec.%5)\nSELECT * FROM Rec";
    QString sqlNormalSelect =
      _db.driver()->sqlStatement(QSqlDriver::SelectStatement, tableName(), patternRecord(), false);
    QString firstRecFilter = initialRecWhere();
    QString where = filter().isEmpty() ? "" : "and " + filter();
    return sqlTemplate.arg(sqlNormalSelect, firstRecFilter, where, _linkField, primaryKeyName(0));
  }
  else {
    qDebug() << "Primary key must be simple and proper linkField should be set";
    return LSqlTableModel::selectAllSql();
  }
}

QString LSqlLinkedRecordsModel::initialRecWhere()
{
  if (_initialLinkValue == QVariant()){
    return Sql::isNull(_linkField);
  }
  else {
    return Sql::concat(_linkField,Sql::concat(Sql::eq(),_initialLinkValue.toString()));
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
