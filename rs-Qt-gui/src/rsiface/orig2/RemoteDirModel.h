#ifndef REMOTE_DIR_MODEL
#define REMOTE_DIR_MODEL

#include <QAbstractItemModel>
#include <vector>

class RemoteDirModel : public QAbstractItemModel
 {
     Q_OBJECT

 public:
     RemoteDirModel(QObject *parent = 0)
         : QAbstractItemModel(parent), nIndex(0) {}

	/* These are all overloaded Virtual Functions */
     int rowCount(const QModelIndex &parent = QModelIndex()) const;
     int columnCount(const QModelIndex &parent = QModelIndex()) const;

     QVariant data(const QModelIndex &index, int role) const;
     QVariant headerData(int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole) const;

     QModelIndex index(int row, int column, 
			const QModelIndex & parent = QModelIndex() ) const;
     QModelIndex parent ( const QModelIndex & index ) const;

     Qt::ItemFlags flags ( const QModelIndex & index ) const;
     bool hasChildren(const QModelIndex & parent = QModelIndex()) const;

	/* Callback from Core */
     void preMods();
     void postMods();

 private:

	class RemoteIndex
	{
		public:
		RemoteIndex() {}
		RemoteIndex(std::string in_person, 
				std::string in_path, 
				int in_idx, 
				int in_row, 
				int in_column)
		:id(in_person), path(in_path), parent(in_idx),
			row(in_row), column(in_column)
		{
			return;
		}
		
		std::string id;
		std::string path;
		int parent;
		int row;
		int column;
	};

     mutable int nIndex;
     mutable std::vector<RemoteIndex> indexSet;
 };

#endif
