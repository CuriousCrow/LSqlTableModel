#ifndef LSTANDARDTREEMODEL_H
#define LSTANDARDTREEMODEL_H

#include <QObject>
#include <QAbstractItemModel>

class LStandardTreeItem : public QObject
{
  Q_OBJECT
public:
  explicit LStandardTreeItem(QObject *parent = 0);
  ~LStandardTreeItem();

  QString caption() const;
  void setCaption(const QString &caption);
private:
  QString _caption;
signals:

public slots:
};


class LStandardTreeModel : public QAbstractItemModel
{
public:
  LStandardTreeModel(QObject *parent = 0);
  ~LStandardTreeModel();

  void addItem(LStandardTreeItem* item, LStandardTreeItem* parent = 0);
  void addItem(LStandardTreeItem* item, QModelIndex parent);
  LStandardTreeItem* itemByIndex(QModelIndex index);
  QModelIndex indexByItem(LStandardTreeItem* item);

  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &child) const;
  virtual int rowCount(const QModelIndex &parent) const;
  virtual int columnCount(const QModelIndex &parent) const;
  virtual bool hasChildren(const QModelIndex &parent) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
  qlonglong genID();

  qlonglong _seqID = 0;

  QHash<qlonglong, LStandardTreeItem*> _itemHash;
  QList<qlonglong> _rootItems;
};

#endif // LSTANDARDTREEMODEL_H
