
#include "RemoteDirModel.h"
#include "rsiface.h"
#include <QPalette>


#include <iostream>
#include <sstream>


 bool RemoteDirModel::hasChildren(const QModelIndex &parent) const
 {
     if (!parent.isValid())
     	return true;

     int idx = parent.internalId();
     if ((idx < 0) || (idx > nIndex-1))
     {
	return false;
     }
     if (indexSet[idx].type == 1) /* TODO */
     	return false;

     return true;
 }

 int RemoteDirModel::rowCount(const QModelIndex &parent) const
 {
     if (!parent.isValid())
     	return 20;

     int idx = parent.internalId();
     if ((idx < 0) || (idx > nIndex-1))
     {
	return 0;
     }
     if (indexSet[idx].type == 1) /* TODO */
     	return 0;

     return indexSet[idx].size;
 }

 int RemoteDirModel::columnCount(const QModelIndex &parent) const
 {
	return 4;
 }

 QVariant RemoteDirModel::data(const QModelIndex &index, int role) const
 {
     if (!index.isValid())
         return QVariant();

     /*
     if (index.row() >= 10)
         return QVariant();
     */

    /* just get the data from the index */
    int idx = index.internalId();
    int coln = index.column(); 

    if ((idx < 0) || (idx > nIndex-1))
    {
	return QVariant();
    }

     if (role == Qt::BackgroundRole)
     {
     	int ts = time(NULL);
     	if (indexSet[idx].timestamp + 30 > ts)
	{
		QBrush brush(QColor(200,250,200));
		return brush;
	}
	else if (indexSet[idx].timestamp + 300 > ts)
	{
		QBrush brush(QColor(200,230,230));
		return brush;
	}
	else
	{
		return QVariant();
	}
     }

     /*************
     Qt::EditRole
     Qt::ToolTipRole
     Qt::StatusTipRole
     Qt::WhatsThisRole
     Qt::SizeHintRole
     ****************/

     if (role == Qt::DisplayRole)
     {

	if (indexSet[idx].type == 2) /* Person */
	{
		switch(coln)
		{
			case 0:
		return QString::fromStdString(indexSet[idx].name);
			break;
			case 1:
		return QString("");
		return QString::fromStdString(indexSet[idx].id);
			break;
			default:
		return QString("");
		return QString::fromStdString("P");
			break;
		}
	}
	else if (indexSet[idx].type == 1) /* File */
	{
		switch(coln)
		{
			case 0:
		return QString::fromStdString(indexSet[idx].name);
			break;
			case 1:
		{
			std::ostringstream out;
			out << indexSet[idx].size;
			return QString::fromStdString(out.str());
		}
			break;
			case 2:
		{
			std::ostringstream out;
			out << indexSet[idx].rank;
			return QString::fromStdString(out.str());
		}
			break;
			case 3:
		{
			std::ostringstream out;
			out << indexSet[idx].timestamp;
			return QString::fromStdString(out.str());
		}
			break;
			default:
		return QString::fromStdString("FILE");
			break;
		}
	}
	else if (indexSet[idx].type == 0) /* Dir */
	{
		switch(coln)
		{
			case 0:
		return QString::fromStdString(indexSet[idx].name);
			break;
			case 1:
		return QString("");
		{
			std::ostringstream out;
			out << indexSet[idx].size;
			return QString::fromStdString(out.str());
		}
			break;
			case 2:
		//return QString::fromStdString(indexSet[idx].path);
		return QString("");
			break;

			default:
		return QString::fromStdString("DIR");
			break;
		}
	}
   } /* end of DisplayRole */
   return QVariant();
 }

 QVariant RemoteDirModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
 {
     if (role == Qt::SizeHintRole)
     {
     	 int defw = 50;
     	 int defh = 30;
     	 if (section < 2)
	 {
	 	defw = 200;
	 }
         return QSize(defw, defh);
     }

     if (role != Qt::DisplayRole)
         return QVariant();

     if (orientation == Qt::Horizontal)
     {
     	switch(section)
	{
		case 0:
			if (RemoteMode)
			{
				return QString("Remote Directories");
			}
			else
			{
				return QString("Local Directories");
			}
			break;
		case 1:
			return QString("Size");
			break;
		case 2:
			return QString("Rank");
			break;
		case 3:
			return QString("Age");
			break;
	}
        return QString("Column %1").arg(section);
     }
     else
         return QString("Row %1").arg(section);
 }

 QModelIndex RemoteDirModel::index(int row, int column,        
                        const QModelIndex & parent) const
 {
	/* create the index */
	int pIdx = -1;
	int itemIdx = 0; /* not allocated yet */
	std::string person;
	std::string path = "";

	/* info to fill in */
	std::string idxName;
	int idxType = 0;
	int idxSize = 0;
	int idxTs   = time(NULL);
	int idxRank = 2; 

	const BaseInfo *bi = NULL; /* for item Model reference. */

	//std::cerr << "RemoteDirModel::index()" << std::endl;

	/* grab data from the rsiface */
	if (!rsiface)
		return QModelIndex();

	rsiface->lockData();   /* Lock Interface */

        if (!parent.isValid())
	{
		//std::cerr << "RemoteDirModel::index() pIdx: " << pIdx;
		//std::cerr << "(" << row << "," << column << ")" << std::endl;
		/* must get the person */
		std::ostringstream out;
		out << "Person:" << row;
		person = out.str();

		std::list<PersonInfo>::const_iterator it, eit;

		const std::list<PersonInfo> *remote = 
				&(rsiface->getRemoteDirectoryList());

		/* Only bit to make it handle both! */

		if (!RemoteMode)
		{
			remote = &(rsiface->getLocalDirectoryList());
		}


		if ((row < 0) || (row >= (signed) remote->size()))
		{
			/* too big */
			//std::cerr << "Return Invalid: Row too Big" << std::endl;
			rsiface->unlockData();   /* Unlock Interface */
			return QModelIndex();
		}

		it = remote->begin();
		eit = remote->end();
		for(int i = 0; i < row; i++, it++); /* iterate through people */
		{
			std::ostringstream out;
			out << it -> id;
			person = out.str();
			idxName = it -> name;
			idxSize = it -> rootdir.subdirs.size();
			idxType = 2;

			bi = &(*it); /* if indexed.. reuse */
		//	std::cerr << "Found Person Entry:" << bi -> mId;
		//	std::cerr << "/" << it -> name << std::endl;
		}
		/* have a pointer to person::baseinfo. */
	}
	else 
	{
		/* get the entry */
		pIdx =  parent.internalId();

		//std::cerr << "RemoteDirModel::index() pIdx: " << pIdx;
		//std::cerr << "(" << row << "," << column << ")" << std::endl;

		if ((pIdx < 0) || (pIdx > nIndex-1))
		{
			std::cerr << "Return Invalid: pIdx Invalid" << std::endl;
			rsiface->unlockData();   /* Unlock Interface */
			return QModelIndex();
		}
		else
		{
			person = indexSet[pIdx].id;
			path   = indexSet[pIdx].path;
		}
		
		/* so now look up the person+dir */
/******************************* FILEY BIT **********************************************/

		std::cerr << "Loading Dir: " << person << "/" << path;
		std::cerr << std::endl;

		//const DirInfo *dir = rsiface->getDirectory(person, path);
		RsCertId rsid(person);
		const DirInfo *dir = rsiface->getDirectory(rsid, path);


		if (!dir)
		{
			std::cerr << "Return Invalid: pIdx Invalid" << std::endl;
			rsiface->unlockData();   /* Unlock Interface */
			return QModelIndex();
		}

		/* find the sub entry -> subdirs, then files */
		/* get the itemIdx if possible */
		if ((row < 0) || (row >= dir->nofiles))
		{
			//error....
			bi = NULL;
			std::cerr << "Row > dir->nofiles. Invalid Index" << std::endl;
			rsiface->unlockData();   /* Unlock Interface */
			return QModelIndex();
		}
		if ((unsigned) row >=  dir->subdirs.size())
		{
			/* file */
			unsigned int fno = row - dir->subdirs.size();
			if (fno >= dir->files.size()) /* then nofiles was wrong */
			{
				std::cerr << "Row > dir-> correct nofiles. Invalid Index" << std::endl;
				rsiface->unlockData();   /* Unlock Interface */
				return QModelIndex();
			}
				
			//std::cerr << "Retrieving File: " << fno << std::endl;
			std::list<FileInfo>::const_iterator it = dir->files.begin();
			for(int i = 0; i < fno; i++, it++);
			bi = &(*it);

			path += "/" + it -> fname;
			//std::cerr << "Name: " << it->fname << std::endl;
			idxName = it -> fname;
			idxSize = it -> size;
			idxType = 1;
			idxRank = it -> rank;
		}
		else
		{
			/* dir */
			//std::cerr << "Retrieving Dir: " << row << std::endl;
			std::list<DirInfo>::const_iterator it = dir->subdirs.begin();
			for(int i = 0; i < row; i++, it++);
			bi = &(*it);

			path += "/" + it -> dirname;
			//std::cerr << "Name: " << it->dirname << std::endl;
			idxName = it -> dirname;
			idxSize = it -> nofiles;
			idxType = 0;
			idxRank = it -> rank;
		}
	}

	/* have valid *bi at this point.
	 * must check if mId valid, and indexed
	 * and do so if not done.
	 */

/******************************* FILEY BIT **********************************************/

	//std::cerr << "Checking Indexing for Entry:" << bi -> mId;
	//std::cerr << std::endl;

	if (bi -> mId > 0)
	{
		/* check old one, and return as good */
		if ((bi -> mId < nIndex) && (indexSet[bi->mId].parent == pIdx))
		{
			//std::cerr << "Found Correct Old Index" << std::endl;
		}

		QModelIndex qmi = createIndex(row, column, bi->mId);
		/* bi->mId); */

		rsiface->unlockData();   /* Unlock Interface */
		return qmi;

		/* else make a new one ... (fall through) should never happen! */
	}

	/* save our index number */
	bi -> mId = nIndex;

	/* create a new one */
	std::cerr << "Index(" << nIndex << ") Creation" << std::endl;
	nIndex++;
	indexSet.resize(nIndex);
	indexSet[nIndex-1] = RemoteIndex(person, path, pIdx, row, column,
                              idxName, idxSize, idxType, idxTs, idxRank);

	std::cerr << "Person:" << person << std::endl;
	std::cerr << "Path:" << path << std::endl;
	std::cerr << "indexSet[].id:" << indexSet[nIndex-1].id << std::endl;

	rsiface->unlockData();   /* Unlock Interface */

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
	//std::cerr << "RemoteDirModel::parent()";
	//std::cerr << std::endl;

	/* create the index */
        if (!index.isValid())
	{
		/* Parent is invalid too */
		return QModelIndex();
	}
	else
	{

	//std::cerr << "RemoteDirModel::parent() Idx: " << index.internalId();
	//std::cerr << std::endl;

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

	//std::cerr << "RemoteDirModel::parent() pIdx: " << pIdx;
	//std::cerr << std::endl;

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
     if (!index.isValid())
	return Qt::ItemIsSelectable; // Error.

     int idx = index.internalId();
     if ((idx < 0) || (idx > nIndex-1))
     {
	return Qt::ItemIsSelectable; // Error.
     }
     if (indexSet[idx].type == 1) /* MAKE INTO DEFINE */
     {
     	/* FILE (No Drop) */
	return 
		( Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsEnabled);
     }
     return 
		( Qt::ItemIsSelectable | 
		Qt::ItemIsDragEnabled |
		Qt::ItemIsDropEnabled |
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
	// nIndex = 0;
	// indexSet.resize(1);
 }

/* Callback from */
 void RemoteDirModel::postMods()
 {
	std::cerr << "RemoteDirModel::postMods()" << std::endl;
	//nIndex = 0;
	//indexSet.resize(1);

	//modelReset();
	layoutChanged();
	//reset();
 }


void RemoteDirModel::update (const QModelIndex &index )
{
	/* grab the controller, 
	 * and ask for update */

	/* */

	/* look up index */
	int mIdx = index.internalId();

	if ((mIdx < 1) || (mIdx >= nIndex))
	{
		std::cerr << "Directory Request() on unknown QMI";
		std::cerr << std::endl;
		return;
	}

	/* if valid */
	/* get id/path */
	std::string id   = indexSet[mIdx].id;
	std::string path = indexSet[mIdx].path;

	std::cerr << "Directory Request(" << id << ") : ";
	std::cerr << path << std::endl;

	rsicontrol -> RequestDirectories(id, path, 1);
}

void RemoteDirModel::downloadSelected(QModelIndexList list)
{
	if (!RemoteMode)
	{
		std::cerr << "Cannot download from local" << std::endl;
	}

	/* so for all the selected .... get the name out, 
	 * make it into something the RsControl can understand
	 */

	/* Fire off requests */
	QModelIndexList::iterator it;
	for(it = list.begin(); it != list.end(); it++)
	{
		unsigned int mIdx = it -> internalId();
		if ((mIdx > 0) && (mIdx < nIndex))
		{
		std::string path = indexSet[mIdx].path;
		std::string id = indexSet[mIdx].id;
		int size       = indexSet[mIdx].size;
		/* only download files */
		if (indexSet[mIdx].type == 1)
			rsicontrol -> FileRequest(id, path, "", size);
		}
	}
}


void RemoteDirModel::recommendSelected(QModelIndexList list)
{
	std::cerr << "recommendSelected()" << std::endl;
	if (RemoteMode)
	{
		std::cerr << "Cannot recommend remote! (should download)" << std::endl;
	}

	/* Print out debugging */
	std::cerr << "Interfacing::::::::::::" << std::endl;
	std::cerr << "RsIface: " << rsiface << std::endl;
	std::cerr << "RsCntrl: " << rsicontrol << std::endl;

	/* so for all the selected .... get the name out, 
	 * make it into something the RsControl can understand
	 */

	std::cerr << "::::::::::::Trying FileRecommend List" << std::endl;
	/* Fire off requests */
	QModelIndexList::iterator it;
	for(it = list.begin(); it != list.end(); it++)
	{
		unsigned int mIdx = it -> internalId();
		std::cerr << "::::::::::::FileRecommend Lookup" << std::endl;
		std::cerr << "mIdx:" << mIdx << std::endl;
		std::cerr << "nIndex:" << nIndex << std::endl;
		if ((mIdx > 0) && (mIdx < nIndex))
		{
		std::string path = indexSet[mIdx].path;
		std::string id = indexSet[mIdx].id;
		int size       = indexSet[mIdx].size;
		std::cerr << "  Id: " << id << std::endl;
		std::cerr << "Path: " << path << std::endl;
		std::cerr << "makeId..." << std::endl;

		/* get the item */
		/* only recommend files */
		if (indexSet[mIdx].type == 1)
		    rsicontrol -> FileRecommend(id, path, size);
		}
	}
	std::cerr << "::::::::::::Done FileRecommend" << std::endl;
}



void RemoteDirModel::openSelected(QModelIndexList list)
{
	if (RemoteMode)
	{
		std::cerr << "Cannot open remote" << std::endl;
		return;
	}

	/* so for all the selected .... get the name out, 
	 * make it into something the RsControl can understand
	 */

	/* Fire off requests */
	QModelIndexList::iterator it;
	for(it = list.begin(); it != list.end(); it++)
	{
		unsigned int mIdx = it -> internalId();
		if ((mIdx > 0) && (mIdx < nIndex))
		{
		std::string path = indexSet[mIdx].path;
		std::string id = indexSet[mIdx].id;
		int size       = indexSet[mIdx].size;

		/* get the item */
		rsicontrol -> FileRecommend(id, path, size);
		}
	}
}



