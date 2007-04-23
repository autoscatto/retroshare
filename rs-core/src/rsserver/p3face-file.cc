
/*
 * "$Id: p3face-file.cc,v 1.6 2007-04-15 18:45:23 rmf24 Exp $"
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2004-2006 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */


#include "rsiface/rsiface.h"
#include "rsserver/p3face.h"
#include "util/rsdir.h"

#include <iostream>
#include <sstream>

#include "pqi/pqidebug.h"
const int p3facefilezone = 11452;

#include <sys/time.h>
#include <time.h>

static const int FileMaxAge = 1800; /* Reload Data after 30 minutes */

/* add in an extension if necessary */
int ensureExtension(std::string &name, std::string def_ext)
{
	return 1;
}


/****************************************/
/****************************************/
int     RsServer::UpdateRemotePeople()
{
        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */

        // get the list of certificates.
        // get the dirlist.
        std::list<DirBase *> &dirs = server -> getDirList();
        std::list<DirBase *>::iterator dit;

        std::list<cert *>::iterator it;
        std::list<cert *> &certs = sslr -> getCertList();

        for(it = certs.begin(); it != certs.end(); it++)
        {
                cert *c = (*it);
                if ((c != NULL) && (c -> Connected()))
                {
                        bool found = false;
                        for(dit = dirs.begin(); dit != dirs.end(); dit++)
                        {
                                if ((*dit) -> p == c)
                                {
                                        found = true;
                                }
                        }
                        if (!found)
                        {
                                // send off searchitem....
                                std::ostringstream out;
                                out << "Generating FileDir Search Item for: " << *it << std::endl;
                                pqioutput(PQL_DEBUG_BASIC, p3facefilezone, out.str());

                                SearchItem *si = new SearchItem(PQI_SI_SUBTYPE_SEARCH);
                                si -> datatype = PQI_SI_DATATYPE_DIR;
                                si -> data = "";
                                // set the target, so it don't go to all!.
                                si -> cid  = c -> cid;
                                si -> p  = c;
                                server -> handleDirectoryRequest(si);
                        }
                }
        }

	/* send off a Directory Request for us */
	if (1)
	{
		// send off searchitem....
		std::ostringstream out;
		out << "Generating FileDir Search Item for: " << *it << std::endl;
		pqioutput(PQL_DEBUG_BASIC, p3facefilezone, out.str());
		
		SearchItem *si = new SearchItem(PQI_SI_SUBTYPE_SEARCH);
		si -> datatype = PQI_SI_DATATYPE_DIR;
		si -> data = "";
		// set the target, so it don't go to all!.

		cert *c = sslr -> getOwnCert();
		si -> cid  = c -> cid;
		si -> p  = c;
		server -> handleDirectoryRequest(si);
	}
		
        /* unlock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

        return 1;
}

#ifdef REMOVE_OLD_CRAPPY_FNS

int     RsServer::UpdateAllFiles()
{
        RsIface &iface = getIface();

        /* Now Notify of Pending Changes */
        NotifyBase &cb = getNotify();
        cb.notifyListPreChange(NOTIFY_LIST_DIRLIST, NOTIFY_TYPE_MOD);

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */

        /* do stuff */
	std::list<DirBase *> &dirs = server -> getDirList();
	std::list<DirBase *>::iterator dit;

	std::list<PersonInfo> &persons = iface.mRemoteDirList;

	persons.clear();

	std::cerr << "RsServer::UpdateAllFiles()" << std::endl;

	for(dit = dirs.begin(); dit != dirs.end(); dit++)
	{
		std::cerr << "RsServer::UpdatePerson()" << std::endl;
		PersonInfo nPerson;
		persons.push_back(nPerson);
		PersonInfo &pref = persons.back();
		initRsPi(*dit, pref);

		UpdateSubdir(*dit, pref.rootdir);
	}

        /* unlock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */


        /* Now Notify of the Change */
        cb.notifyListChange(NOTIFY_LIST_DIRLIST, NOTIFY_TYPE_MOD);
        return 1;
}

/* Sub Function - Must be under locks... */
int     RsServer::UpdateSubdir(DirNode * dir, DirInfo &di)
{
        std::list<PQFileItem *> &files = dir->files;
        std::list<DirNode *> &dirs = dir->subdirs;
        std::list<PQFileItem *>::iterator fit;
        std::list<DirNode *>::iterator dit;

        {
          std::ostringstream out;
          out << "RsServer::UpdateSubdir(" << dir -> name << ")";
          pqioutput(PQL_DEBUG_BASIC, p3facefilezone, out.str());
        }

	std::cerr << "RsServer::UpdateSubdir()" << std::endl;

        for(dit = dirs.begin(); dit != dirs.end(); dit++)
        {
		/* add subdir */
		DirInfo sdi;
		di.subdirs.push_back(sdi);
		DirInfo &rsdi = di.subdirs.back();
		/* fill data */
		initRsDi(*dit, rsdi);
		/* process subdirs */
		UpdateSubdir(*dit, rsdi);
        }

        for(fit = files.begin(); fit != files.end(); fit++)
        {
		FileInfo fi;
		initRsFi(*fit, fi);
		di.files.push_back(fi);
        }
	return 1;
}

int     RsServer::initRsPi(DirBase *dir, PersonInfo &pi)
{
	/* do the data */
	pi.id = intGetCertId((cert *) (dir -> p));
	pi.name = dir -> p -> Name(); 
	return 1;
}

int     RsServer::initRsDi(DirNode *dir, DirInfo &di)
{
	/* do the data */
	di.dirname = dir -> name; 
	return 1;
}

int     RsServer::initRsFi(PQFileItem *file, FileInfo &fi)
{
	/* do the data */
	fi.fname = file -> name;
	fi.size  = file -> size;
	fi.inRecommend = false;
	return 1;
}

#endif /* REMOVE_OLD_CRAPPY_FNS */

/****************************************/
/****************************************/
int     RsServer::UpdateAllTransfers()
{
        NotifyBase &cb = getNotify();
        cb.notifyListPreChange(NOTIFY_LIST_TRANSFERLIST, NOTIFY_TYPE_MOD);

        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */


	/* clear the old transfer list() */
	std::list<FileTransferInfo> &transfers = iface.mTransferList;
	transfers.clear();

	std::list<FileTransferItem *> nTransList = server -> getTransfers();
	std::list<FileTransferItem *>::iterator it;

	for(it = nTransList.begin(); it != nTransList.end(); it++)
	{
		FileTransferInfo ti;
		if ((*it) -> p)
		{
			ti.source = (*it) -> p -> Name();
		}
		else
		{
			ti.source = "Unknown";
		}
	
		ti.id = intGetCertId((cert *) (*it) -> p);
		ti.fname = (*it) -> name;
		ti.path  = (*it) -> path;
		ti.size  = (*it) -> size;
		ti.transfered = (*it) -> transferred;
		/* other ones!!! */
		ti.tfRate = (*it) -> crate;
		ti.downloadStatus = (*it) -> state;
		transfers.push_back(ti);
	}

        iface.setChanged(RsIface::Transfer);

        /* unlock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

        /* Now Notify of the Change */
        cb.notifyListChange(NOTIFY_LIST_TRANSFERLIST, NOTIFY_TYPE_MOD);

        return 1;
}


        /* Directory Actions */
int RsServer::DirectoryRequestUpdate(RsCertId uId, std::string dir)
{
	lockRsCore(); /* LOCKED */

	std::cerr << "RsServer::DirectoryRequestUpdate()" << std::endl;

	int ret = 1;
	cert *c = intFindCert(uId);
	if (c == sslr -> getOwnCert())
	{
		/* okay ... can use own cert! (loopback) */
	}
	else if ((c == NULL) || (!(c->Connected())))
	{
		ret = 0;
		std::cerr << "RsServer::DirectoryRequestUpdate() Invalid Cert";
		std::cerr << std::endl;
	}

	if (ret)
	{
		/* send off request */
		std::ostringstream out;
                out << "Generating FileDir Search Item for: " << c -> certificate -> name << std::endl;
                out << "Directory: " << dir << std::endl;

                SearchItem *si = new SearchItem(PQI_SI_SUBTYPE_SEARCH);
                si -> datatype = PQI_SI_DATATYPE_DIR;
                si -> data = dir;
                si -> cid  = c -> cid;
                si -> p    = c;
                server -> handleDirectoryRequest(si);

		std::cerr <<  out.str() << std::endl;
                //pqioutput(PQL_DEBUG_BASIC, fltksrvrzone, out.str());
        }

	unlockRsCore(); /* UNLOCKED */

	return ret;
}


/**
 * int RsServer::FileRequest(FileItem &from, FileItem &dest)
 **/

int RsServer::FileRequest(std::string id, std::string src, std::string dest, int size)
{
	lockRsCore(); /* LOCKED */
	RsCertId uId(id);

	std::cerr << "RsServer::FileRequest(" << uId << ", ";
	std::cerr << src << ", " << dest << ")" << std::endl;

	int ret = 1;
	cert *c = intFindCert(uId);
	if ((c == NULL) || (c == sslr -> getOwnCert()))
	{
		ret = 0;
		std::cerr << "RsServer::FileRequest() Bade Cert!" << std::endl;
	}

	if (ret)
	{
		// XXX Must construct this properly.
		PQFileItem *result_item = new PQFileItem();

		//intFillFileItem(c, fname, result_item);
		result_item -> p = c;
		result_item -> cid = c -> cid;
		result_item -> subtype = PQI_FI_SUBTYPE_REQUEST;

		std::string basepath = RsDirUtil::removeTopDir(src);
		std::string fname = RsDirUtil::getTopDir(src);

		result_item -> path = basepath;
		result_item -> name = fname;
		result_item -> size = size;
		/* what about the size....? */

		// otherwise ... can get search item.
		std::cerr << "RsServer::FileRequest() Requesting:" << std::endl;
		result_item -> print(std::cerr);

		server -> getSearchFile(result_item);
	}

	unlockRsCore(); /* UNLOCKED */

	return ret;
}


        /* Actions For Upload/Download */
int RsServer::FileRecommend(std::string id, std::string src, int size)
{
	/* check for new data */
        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */
	RsCertId uId(id);

	int ret = 1;
	cert *c = intFindCert(uId);

	if (c == NULL) // || (c == sslr -> getOwnCert()))
	{
		ret = 0;
		std::cerr << "RsServer::FileRecommend(Failed for bad Cert)";
		std::cerr << std::endl;
	}

	if (ret)
	{
		/* add the entry to FileRecommends.
		 */

		std::list<FileInfo> &recList = iface.mRecommendList;
		std::list<FileInfo>::iterator it;

		FileInfo fi;
		fi.path = src;
		fi.fname = RsDirUtil::getTopDir(src);
		fi.size = size;
		fi.rank = 1;
		fi.inRecommend = false;

		/* check if it exists already! */
		bool found = false;
		for(it = recList.begin(); (!found) && (it != recList.end()); it++)
		{
			if ((it->path == fi.path) && (it -> fname == fi.fname))
			{
				found = true;
			}
		}

		if (!found)
		{
			recList.push_back(fi);
		}
	}

	/* Notify of change */
	iface.setChanged(RsIface::Recommend);

        /* lock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

	return ret;
}


int RsServer::FileBroadcast(std::string id, std::string src, int size)
{
	lockRsCore(); /* LOCKED */
	RsCertId uId(id);

	int ret = 1;
	cert *c = intFindCert(uId);
	if ((c == NULL) || (c == sslr -> getOwnCert()))
	{
		ret = 0;
	}

	if (ret)
	{
		/* TO DO */
	}

	unlockRsCore(); /* UNLOCKED */

	return ret;
}

int RsServer::FileCancel(std::string id, std::string fname)
{
	lockRsCore(); /* LOCKED */
	RsCertId uId(id);

	int ret = 1;
	cert *c = intFindCert(uId);
	if ((c == NULL) || (c == sslr -> getOwnCert()))
	{
		ret = 0;
	}

	if (ret)
	{
		/* TO DO */
        	PQFileItem *cancelled = new PQFileItem();
		cancelled -> name = fname;
		cancelled -> hash = "";
		server -> cancelTransfer(cancelled);
	}

	unlockRsCore(); /* UNLOCKED */

	return ret;
}


int RsServer::FileClearCompleted()
{
	std::cerr << "RsServer::FileClearCompleted()" << std::endl;

	/* check for new data */
        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */

	server -> clear_old_transfers();

        /* lock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

	/* update data */
	UpdateAllTransfers();

	return 1;
}


int RsServer::FileSetBandwidthTotals(float outkB, float inkB)
{
	int ret = 0;
	return ret;
}

int RsServer::FileDelete(std::string id, std::string fname)
{
	lockRsCore(); /* LOCKED */
	RsCertId uId(id);

	int ret = 1;
	cert *c = intFindCert(uId);
	if ((c == NULL) || (c == sslr -> getOwnCert()))
	{
		ret = 0;
	}

	if (ret)
	{
		/* TO DO */
	}

	unlockRsCore(); /* UNLOCKED */

	return ret;
}

void intFillFileItem(cert *c, FileInfo &fi, PQFileItem *item)
{
	item -> cid = c -> cid;
	//item -> path = fname;

	return;
}

/* NOTEs on the directory Listing .....
 *
 * Full tree maintained in RsIface....
 *
 * Info:
 * 	Dir -> 
 * 	(1)     no-entries (direct sub entries dirs + files)
 * 	(2) 	no-bytes   (total)
 * 	(3)	timestamp - update (local time stamp)
 * 	(4)	timestamp - last mod.
 *      (5)	retrived or not.
 *
 *
 * 	Update is RootDir + Entries.
 *          this is collected together...
 *		
 *      Regular Check of the root dir... (10-30 minutes)
 * 	    if last-mod is more recent than update....
 *		then request it again.
 *
 *
 *  When request directory (from GUI)
 *       store directory + Flag 
 *		-1 - all subdirs.
 * 		0 - just directory
 * 		1 - directory + subdirs
 *              n - dir + n subdirs.
 *
 *       add entries as they arrive
 *           if get subdir (>0 or mod) ... add new request dir.
 *       when complete... update tree + GUI. (notify(updated dir))
 *  if timeout... delete dir + add entry.
 *
 *  Dir
 *    either 
 *    (1) Loaded
 *    (2) ToLoad
 *    (3) Unavailable
 *    (4) Empty.
 * 
 *  if recieve Dir Entry....
 *        
 *  Jane
 *      dirA
 *        dirB
 *          <ToLoad>
 *        dirC
 *          File1
 *          File2
 *        File3
 *        File4
 *
 *     recv DirA (updated)
 *          -> trigger DirA request.
 *               recv DirB (updated)
 *                -> update entry, but no sub req (as to load).
 *               recv DirC (updated)
 *                -> trigger update. (etc).
 *
 *
 */


//int	breakupDirlist(std::string path, std::list<std::string> &subdirs)


/* Called if pending data */
int RsServer::UpdateDirectories()
{
	std::cerr << "RsServer::UpdateDirectories()" << std::endl;

	/* check for new data */
        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */

	PQFileItem *item = NULL;
	/* continue while more data items */
	while(NULL != (item = server->getDirPassThrough()))
	{
		std::cerr << "RsServer::UpdateDirectories() Recieved Item" << std::endl;
		/* must be Locked Here */
		/* match up to request */

		std::list<PendingDirectory>::iterator it;
		RsCertId uid = intGetCertId((cert *) (item -> p));
		std::string path = item -> path;

		std::string uid_str;
		{
			std::ostringstream out;
			out << uid;
			uid_str = out.str();
		}

		/* got item */
		/* find the pending dir */
		bool found = false;
		for(it = pendingDirs.begin(); 
			(!found) && (it != pendingDirs.end()); it++)
		{
			std::string pdir = it -> data.path + "/" + it->data.dirname;
			/* match ids + paths */
			if ((it -> id == uid) && 
				(pdir == item -> path))
			{
				found = true;
				break;
			}
		}

		if (found)
		{
			std::cerr << "RsServer::UpdateDirectories() Matched: ";
			std::cerr << item->path << " with existing Req - Adding" << std::endl;
			/* add to the Pending Directory Request */
			it -> addEntry(item);
			/*  ...  */
		}
		else
		{
			std::cerr << "RsServer::UpdateDirectories() UnMatched Response: ";
			std::cerr << item->path << std::endl;
			std::cerr << "RsServer::UpdateDirectories() Temp Hack Adding pending.";
			std::cerr << std::endl;

			std::list<PersonInfo> *persons = &(iface.mRemoteDirList);
			if (item -> p == sslr -> getOwnCert())
			{
				persons = &(iface.mLocalDirList);
			}

			const PersonInfo *pi = iface.getPerson(uid_str);
			if (!pi) /* add it in! */
			{
				std::cerr << "RsServer::UpdateDirectories() addPerson()" << std::endl;
				PersonInfo nPerson;
				persons -> push_back(nPerson);
				PersonInfo &pref = persons -> back();
				/* fill it in */
				pref.id = uid;
				pref.name = item -> p -> Name(); 
				pref.infoAge = time(NULL);
				pref.online = true;
				/* fill in root directory */
				pref.rootdir.path = ""; // or "/";
				pref.rootdir.dirname = ""; // or "/"; //???
				pref.rootdir.infoAge = pref.infoAge;
				pref.rootdir.nofiles = 0;
				pref.rootdir.nobytes = 0;
				/* no need to clear the lists */
			}

			const DirInfo *dir = iface.getDirectory(uid_str, item->path);
			if (dir)
			{
				pendingDirs.push_front(PendingDirectory(uid, dir, 1));
				it = pendingDirs.begin();
				found = true;
				it -> addEntry(item);
				it -> data.nofiles = 1; /* hack the directory count to accept now. */
			}
			else
			{
				std::cerr << "RsServer::UpdateDirectories() Failed Temp Hack.";
				std::cerr << std::endl;
			}
		}

		/* clean up item */
		delete item;

		/* if pendingDir is complete */
		if ((found) && (DirectoryUpToDate(&(it -> data))))
		{

			std::cerr << "RsServer::UpdateDirectories() PendingDir UpToDate .... Updating Tree!";
			std::cerr << std::endl;


			/* pop data out -> local stack */
			PendingDirectory pd = (*it);
			pendingDirs.erase(it);

			/* determine entry point in dir tree */
			const DirInfo *dir = iface.getDirectory(uid_str, path);
			/* check if they check if they are there already 
			 * TODO 
			 */


			/* find what we're adding */

			dir = NULL; /* cannot use this again! */
			
        		/* unlock Mutexes */
        		iface.unlockData(); /* UNLOCK */
        		unlockRsCore();     /* UNLOCK */

			/* Notify of Pending Update */
			std::cerr << "RsServer::UpdateDirectories() Should Notify of Pending update!";
			std::cerr << std::endl;
			{
        			NotifyBase &cb = getNotify();
        			cb.notifyListPreChange(NOTIFY_LIST_DIRLIST, NOTIFY_TYPE_MOD);
			}

        		/* lock Mutexes */
        		lockRsCore();     /* LOCK */
        		iface.lockData(); /* LOCK */

			std::cerr << "RsServer::UpdateDirectories() Adding Data to Tree!";
			std::cerr << std::endl;

			/* Perform update (protected call - friend class) */
			DirInfo *dirup = iface.getDirectoryMod(uid_str, path);

			/* add the info */
			dirup -> merge(pd.data);


        		/* unlock Mutexes */
        		iface.unlockData(); /* UNLOCK */
        		unlockRsCore();     /* UNLOCK */

			std::cerr << "RsServer::UpdateDirectories() Notifying of Change!";
			std::cerr << std::endl;


			/* Notify of Completed Update */
		        /* Now Notify of the Change */
			/* OLD NOTIFiCATION - FULL REDRAW!!! */
			{
        			NotifyBase &cb = getNotify();
        			cb.notifyListChange(NOTIFY_LIST_DIRLIST, NOTIFY_TYPE_MOD);
			}

			std::cerr << "RsServer::UpdateDirectories() Checking Directory";
			std::cerr << std::endl;
 
			/* RequestDirectory (do orig...
			 *   will automatically do subdirs) 
			 */
			//RequestDirectories(pd.id, pd.data.path, pd.reqDepth);
			{
				std::ostringstream out;
				out << pd.id;
				RequestDirectories(out.str(), pd.data.path, pd.reqDepth);
			}

        		/* lock Mutexes */
        		lockRsCore();     /* LOCK */
        		iface.lockData(); /* LOCK */

			std::cerr << "RsServer::UpdateDirectories() PendingDir UpToDate .... Stage 1Ending...";
			continue;
		}
		/* must be Locked Here */
	}

	/* Print out PendingDirs.... */

	std::list<PendingDirectory>::iterator it;

	std::cerr << "Pending Dir Count: " << pendingDirs.size() << std::endl;
	for(it = pendingDirs.begin(); it != pendingDirs.end(); it++)
	{
		std::string pdir = it -> data.path + "/" + it->data.dirname;

		std::cerr << "Pending Dir: " << pdir << std::endl;
		std::cerr << "\tId: " << it -> id;
		std::cerr << std::endl;
		std::cerr << "\tAge: " << time(NULL) - it -> reqTime;
		std::cerr << std::endl;
		std::cerr << "\tPath: " << it -> data.path;
		std::cerr << std::endl;
	}






        /* unlock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

	std::cerr << "RsServer::UpdateDirectories() Done";
	std::cerr << std::endl;
	return 1;
}



int RsServer::RequestDirectories(std::string uid_str, std::string path, int depth)
{
	RsCertId uid(uid_str);
	std::cerr << "RsServer::RequestDirectories()";
	std::cerr << " UID: " << uid;
	std::cerr << " Path: " << path;
	std::cerr << " Depth: " << depth;
	std::cerr << std::endl;

        RsIface &iface = getIface();

        /* lock Mutexes */
        lockRsCore();     /* LOCK */
        iface.lockData(); /* LOCK */


	/* firstly check for existing query */

	std::list<PendingDirectory>::iterator it;
	/* got item */
	/* find the pending dir */
	bool found = false;
	for(it = pendingDirs.begin(); 
		(!found) && (it != pendingDirs.end()); it++)
	{
		std::string pdir = it -> data.path + "/" + it->data.dirname;
		/* match ids + paths */
		if ((it -> id == uid) && 
			(pdir == path))
		{
			found = true;
			break;
		}
	}

#define MIN_DUP_REQ_TIME 30

	if ((found) && (it -> reqTime < time(NULL) + MIN_DUP_REQ_TIME))
	{
		std::cerr << "RsServer::RequestDirectories() Found existing query";
		std::cerr << std::endl;

		if (it -> reqDepth < depth)
		{
			std::cerr << "RsServer::RequestDirectories() Updating depth to: " << depth;
			std::cerr << std::endl;
			it -> reqDepth = depth;
		}

        	/* unlock Mutexes */
        	iface.unlockData(); /* UNLOCK */
        	unlockRsCore();     /* UNLOCK */

		return 0;
	}
	else if (found)
	{
		/* clean up */
		pendingDirs.erase(it);
	}

	/* locate directory (const) */
	const DirInfo *dir = iface.getDirectory(uid_str, path);

	if (!dir)
	{
        	/* unlock Mutexes */
        	iface.unlockData(); /* UNLOCK */
        	unlockRsCore();     /* UNLOCK */

		return 0;
	}
	
	if (DirectoryUpToDate(dir))
	{
		/* handle subdirectory */
		std::cerr << "RsServer::RequestDirectories() Dir UpToDate -> Checking Subdirs...";
		std::cerr << std::endl;

		int subdepth = depth - 1;
		std::list<std::string> subdirs;
		std::list<std::string>::iterator sdit;
		if ((depth == -1) || (subdepth > 0))
		{
			/* make stack copy of directories */
			std::list<DirInfo>::const_iterator it;
			for(it = dir->subdirs.begin(); 
				it != dir->subdirs.end(); it++)
			{
				subdirs.push_back(it->path + "/" + it -> dirname);
			}
			if (depth == -1) 
				{ subdepth = -1; }
		}

        	/* unlock Mutexes */
        	iface.unlockData(); /* UNLOCK */
        	unlockRsCore();     /* UNLOCK */

		/* iterate through the subdirs - recursively */
		for(sdit = subdirs.begin(); sdit != subdirs.end(); sdit++)
		{
			//RequestDirectories(uid, *sdit, subdepth);
			{
				std::ostringstream out;
				out << uid;
				RequestDirectories(out.str(), *sdit, subdepth);
			}
		}
		return 1;
	}

	/* if not complete...
	 * add to incoming queue:
	 * First look it up and get the details from the Tree.
	 */
	
	std::cerr << "RsServer::RequestDirectories() Adding PendDir Req: " << path;
	std::cerr << std::endl;
	
	pendingDirs.push_back(PendingDirectory(uid, dir, depth));

        /* unlock Mutexes */
        iface.unlockData(); /* UNLOCK */
        unlockRsCore();     /* UNLOCK */

	/* do request */
	return DirectoryRequestUpdate(uid, path);
	
}



bool RsServer::DirectoryUpToDate(const DirInfo *dir)
{
	std::cerr << "RsServer::DirectoryUpToDate() ... Checking:";
	std::cerr << std::endl;
	//print(std::cerr, (*dir) , 0);
	std::cerr << std::endl;

	if (!dir)
	{
		std::cerr << "RsServer::DirectoryUpToDate() Fail NULL";
		std::cerr << std::endl;
		
		return false;
	}

	/* upToDate, if
	 * (1) Timestamp < minimum
	 * (2) nofiles = no-dirs + no-files.
	 * (3) Timestamps of Children are up-to-date
	 */
	time_t ts = time(NULL);
	if (ts - dir->infoAge > FileMaxAge)
	{
		std::cerr << "RsServer::DirectoryUpToDate() Fail Dir Age";
		std::cerr << " TS: " << ts << " iA: " << dir->infoAge << " diff: ";
		std::cerr << ts - dir->infoAge;
		std::cerr << std::endl;
		return false;
	}

	if ((unsigned) dir->nofiles != dir->subdirs.size() + dir->files.size())
	{
		std::cerr << "RsServer::DirectoryUpToDate() Fail File Count";
		std::cerr << std::endl;
		std::cerr << "Expected: " << dir->nofiles << " Have: " << dir->subdirs.size() + dir->files.size();
		std::cerr << std::endl;
		return false;
	}

	std::list<DirInfo>::const_iterator it;
	for(it = dir->subdirs.begin(); it != dir->subdirs.end(); it++)
	{
		if (ts - (*it).infoAge > FileMaxAge)
		{
			std::cerr << "RsServer::DirectoryUpToDate() Fail Subdir Age";
			std::cerr << std::endl;
			return false;
		}
	}

	/**** TS only on the Directories *******
	std::list<FileInfo>::const_iterator fit;
	for(fit = dir->files.begin(); fit != dir->files.end(); fit++)
	{
		if (ts - (*fit).infoAge > FileMaxAge)
		{
			return false;
		}
	}
	******************/

	return true;
}

PendingDirectory::PendingDirectory(RsCertId in_id, DirInfo const *in_dir, int in_depth)
	:id(in_id), reqDepth(in_depth)
{
	reqTime = time(NULL);

	/* fill in the data ... */
	data.path    = in_dir->path;
	data.dirname = in_dir->dirname;
	data.infoAge = reqTime; //in_dir->infoAge;
	data.nofiles = in_dir->nofiles;
	data.nobytes = in_dir->nobytes;


	std::cerr << "PendingDirectory::PendingDirectory() for: ";
	std::cerr << data.path << " + " << data.dirname;
	std::cerr << std::endl;
	std::cerr << "Expecting " << data.nofiles << " Entries";
	std::cerr << std::endl;
	std::cerr << "InfoAge: " << data.infoAge;
	std::cerr << std::endl;
}

void PendingDirectory::addEntry(PQFileItem *i)
{
	std::cerr << "PendingDirectory::addEntry()" << std::endl;

	/**/
	if (i -> reqtype == PQIFILE_DIRLIST_FILE)
	{
		/* if its a file ... */
		FileInfo fi;

		fi.path = i->path;
		fi.fname = i->name;
		fi.size =  i->size;
		fi.avail = i->fileoffset;
		fi.rank = i->chunksize;
		fi.inRecommend = false;

		//fi.infoAge = time(NULL);

		data.add(fi);
		//data.files.push_back(fi);
		std::cerr << "PendingDirectory::addEntry() Adding File: " << i->name << std::endl;

	}
	else
	{
		DirInfo di;
		/* fill in the data ... */
		di.path    = i->path;
		di.dirname = i->name;
		di.infoAge = time(NULL);
		di.nofiles = i->size;

		data.add(di);
		//data.subdirs.push_back(di);
		std::cerr << "PendingDirectory::addEntry() Adding Dir: " << i->name << std::endl;
	}
}


