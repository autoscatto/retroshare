
#include "RemoteDirModel.h"
#include "rsiface.h"

#include <iostream>
#include <sstream>


 bool RemoteDirModel::hasChildren(const QModelIndex &parent) const
 {
	return true;
 }

 int RemoteDirModel::rowCount(const QModelIndex &parent) const
 {
     return 10;
 }

 int RemoteDirModel::columnCount(const QModelIndex &parent) const
 {
	return 1;
 }

 QVariant RemoteDirModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     if (index.row() >= 10)
         return QVariant();

     if (role == Qt::DisplayRole)
     {
	/* just get the data from the index */
	int idx = index.internalId();
	if ((idx < 0) || (idx > nIndex-1))
	{
		return QVariant();
	}
	
	if (indexSet[idx].path == "")
	{
		return QString::fromStdString(indexSet[idx].id);
	}
	else
	{
		return QString::fromStdString(indexSet[idx].path);
	}
     }
     else
         return QVariant();
 }

 QVariant RemoteDirModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
 {
     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal)
         return QString("Column %1").arg(section);
     else
         return QString("Row %1").arg(section);
 }


 QModelIndex RemoteDirModel::index(int row, int column,        
                        const QModelIndex & parent) const
 {
	/* create the index */
	int pIdx = -1;
	std::string person;
	std::string path;

	std::cerr << "RemoteDirModel::index()" << std::endl;

        if (!parent.isValid())
	{
		/* must get the person */
		std::ostringstream out;
		out << "Person:" << row;
		person = out.str();

		/* grab data from the rsiface */
		if (!rsiface)
			return QModelIndex();

		rsiface->lockData();   /* Lock Interface */
		std::list<PersonInfo>::const_iterator it, eit;
		const std::list<PersonInfo> &remote = 
				rsiface->getRemoteDirectoryList();

		if (row >= (signed) remote.size())
		{
			/* too big */
			return QModelIndex();
		}
		it = remote.begin();
		eit = remote.end();
		for(int i = 0; i < row; i++, it++); /* iterate through people */

		{
			std::ostringstream out;
			out << it -> id;
			person = out.str();
		}
		rsiface->unlockData();   /* Lock Interface */
	}
	else 
	{
		/* get the entry */
		pIdx =   parent.internalId();

		std::cerr << "Index Parent: " << pIdx << std::endl;
		if ((pIdx < 0) || (pIdx > nIndex-1))
		{
			std::cerr << "Invalid Index Parent" << std::endl;
		}
		else
		{
			person = indexSet[pIdx].id;
			path   = indexSet[pIdx].path;
		}

		/* now add new directory/file */
		std::ostringstream out;
		out << "/Dir-" << row;
		path += out.str();

		/* iterate through the dirs */
	}

	/* create a new one */
	std::cerr << "Index(" << nIndex << ") Creation" << std::endl;

	nIndex++;
	indexSet.resize(nIndex);
	indexSet[nIndex-1] = RemoteIndex(person, path, pIdx, row, column);


	/* look up parent in internal index */
	std::cerr << "Index(" << nIndex-1 << ") Creation:";
	std::cerr << row << ", " << column << ", " << nIndex-1;
 	std::cerr << std::endl;

	QModelIndex qmi = createIndex(row, column, nIndex-1);

	std::cerr << "Internal Id: " << qmi.internalId();
	std::cerr << std::endl;

	return qmi;
 }


 QModelIndex RemoteDirModel::parent( const QModelIndex & index ) const
 {
	std::cerr << "RemoteDirModel::parent()";
	std::cerr << std::endl;

	/* create the index */
        if (!index.isValid())
	{
		/* Parent is invalid too */
		return QModelIndex();
	}
	else
	{

	std::cerr << "RemoteDirModel::parent() Idx: " << index.internalId();
	std::cerr << std::endl;

		/* get the entry */
		int idx  = index.internalId();

		if ((idx < 0) || (idx > nIndex-1))
		{
	std::cerr << "RemoteDirModel::parent() Invalid Index";
	std::cerr << std::endl;
			/* Parent is Invalid */
			return QModelIndex();
		}
		int pIdx = indexSet[idx].parent;

	std::cerr << "RemoteDirModel::parent() pIdx: " << pIdx;
	std::cerr << std::endl;

		if ((pIdx < 0) || (pIdx > nIndex-1))
		{
	std::cerr << "RemoteDirModel::parent() Invalid Parent";
	std::cerr << std::endl;
			/* Parent is Invalid */
			return QModelIndex();
		}

		int row     = indexSet[pIdx].row;
		int column  = indexSet[pIdx].column;

		return createIndex(row, column, pIdx);
	}
 }

 Qt::ItemFlags RemoteDirModel::flags( const QModelIndex & index ) const
 {
	return 
		( Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsEnabled);

// The other flags...
//Qt::ItemIsUserCheckable
//Qt::ItemIsEditable
//Qt::ItemIsDropEnabled
//Qt::ItemIsTristate

 }

/* Callback from */
 void RemoteDirModel::preMods()
 {
	std::cerr << "RemoteDirModel::preMods()" << std::endl;
	//modelAboutToBeReset();
	layoutAboutToBeChanged();

	/* clear all indices */
	nIndex = 0;
	indexSet.resize(1);
 }

/* Callback from */
 void RemoteDirModel::postMods()
 {
	std::cerr << "RemoteDirModel::postMods()" << std::endl;
	nIndex = 0;
	indexSet.resize(1);

	//modelReset();
	layoutChanged();
	reset();
 }

