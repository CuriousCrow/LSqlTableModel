#include "lstandardtreemodel.h"
#include <QDebug>

LStandardTreeItem::LStandardTreeItem(QObject *parent) : QObject(parent)
{
  qDebug() << "item" << this << "created";
}

LStandardTreeItem::~LStandardTreeItem()
{
  qDebug() << "item" << this << "destroyed";
}
QString LStandardTreeItem::caption() const
{
  return _caption;
}

void LStandardTreeItem::setCaption(const QString &caption)
{
  _caption = caption;
}


LStandardTreeModel::LStandardTreeModel(QObject *parent): QAbstractItemModel(parent)
{

}

LStandardTreeModel::~LStandardTreeModel()
{

}

void LStandardTreeModel::addItem(LStandardTreeItem *item, LStandardTreeItem *parent)
{  
  addItem(item, indexByItem(parent));
}

void LStandardTreeModel::addItem(LStandardTreeItem *item, QModelIndex parent)
{
  int newRowIndex = rowCount(parent);
  beginInsertRows(parent, newRowIndex, newRowIndex);
  qlonglong itemId = genID();
  if (parent.isValid()){
    item->setParent(itemByIndex(parent));
  }
  else {
    item->setParent(this);
    _rootItems.append(itemId);
  }
  _itemHash.insert(itemId, item);
  endInsertRows();
}

LStandardTreeItem *LStandardTreeModel::itemByIndex(QModelIndex index)
{
  return _itemHash[index.internalId()];
}

QModelIndex LStandardTreeModel::indexByItem(LStandardTreeItem *item)
{
  qlonglong intId = _itemHash.key(item, -1);
  if (intId < 0)
    return QModelIndex();

  QObject* parentObj = item->parent();
  int itemRow = parentObj->children().indexOf(item);
  return createIndex(itemRow, 0, intId);
}

QModelIndex LStandardTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  if (!hasIndex(row, column, parent))
    return QModelIndex();

  qlonglong childId = 0;

  //Если родительский индекс невалидный, значит это корневой элемент
  if (!parent.isValid()){
    childId = _rootItems.at(row);
  }
  //получаем элемент родителя и по номеру элемента получает ID
  else {
    LStandardTreeItem* parentItem = _itemHash[parent.internalId()];
    LStandardTreeItem* childItem = (LStandardTreeItem*)parentItem->children().at(row);
    childId = _itemHash.key(childItem);
  }
  if (childId){
    return createIndex(row, column, childId);
  }
  else {
    return QModelIndex();
  }
}

QModelIndex LStandardTreeModel::parent(const QModelIndex &child) const
{
  LStandardTreeItem* childItem = _itemHash[child.internalId()];  
  if (childItem->parent()->inherits("LStandardTreeItem")){
    LStandardTreeItem* parentItem = (LStandardTreeItem*)childItem->parent();
    QObject* grandParent = parentItem->parent();
    return createIndex(grandParent->children().indexOf(parentItem), 0, _itemHash.key(parentItem));
  }
  else {
    return QModelIndex();
  }
}

int LStandardTreeModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid()){
    LStandardTreeItem* parentItem = _itemHash.value(parent.internalId());
    int childCount = parentItem->children().count();
    return childCount;
  }
  else {
    return _rootItems.count();
  }
}

int LStandardTreeModel::columnCount(const QModelIndex &parent) const
{
  return 1;
}

bool LStandardTreeModel::hasChildren(const QModelIndex &parent) const
{
  return rowCount(parent) > 0;
}

QVariant LStandardTreeModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();
  LStandardTreeItem* item = _itemHash[index.internalId()];
  if (role == Qt::DisplayRole){
//    qDebug() << "data for index" << index.row() << index.internalId();
    return item->caption();
  }
  else {
    return QVariant();
  }
}

bool LStandardTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
  //
}

qlonglong LStandardTreeModel::genID()
{
  return ++_seqID;
}
