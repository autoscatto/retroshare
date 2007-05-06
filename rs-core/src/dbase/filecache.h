/*
 * "$Id: filecache.h,v 1.1 2007-02-19 20:08:30 rmf24 Exp $"
 *
 * Other Bits for RetroShare.
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


#ifndef _PQI_FILECACHE_H
#define _PQI_FILECACHE_H

/* 
 *
 * This class provides the File Cache. This is an 
 * intermediate Interface, that is used by retroshare
 * to do searches and file lookups.
 *
 * RetroShare
 *  |  / \
 *  |   |
 * \ /  |
 * FileCache -> Persistent storage. 
 *  |  / \
 *  |   |
 * \ /  |
 * FileLook
 *
 * The FileLook Layer provides file/directory lookup.
 * but it is not able to handle searches. The 
 * FileCache's role will be to maintain a list
 * of the most popular / most recent files.
 * 
 * Its basic behaviour would be:
 *
 * (A) recvs a SearchItem.
 *     - search Through Cache for matches.
 *     - filter results based on GroupSettings.
 *     - update the Cache with hits (Popularity)
 *     - put the results in Outgoing Queue.
 *
 * (B) recvs FileItem.
 *     - Pass to FileLook to lookup
 *     - filter results based on GroupSettings.
 *     - update the Cache with the results.
 *     - update results with Cached Data. (Popularity/File Timestamp)
 *     - put results in the Outgoing Queue.
 */

#include <list>
#include <map>

#include "pqi/pqi.h"
#include "util/rsthreads.h"


class fileCache: public RsThread
{
	public:

	fileCache();
virtual ~fileCache();

	/* threading */
virtual void	run(); /* overloaded */

	/* fileLook interface */

	/* Input to fileCache */
int	    search(PQItem *i);
int	    lookUpDirectory(PQItem *i);
PQFileItem *findFileEntry(PQFileItem *);

	/* Output from fileCache */
PQItem  *getResult();


void	setSharedDirectories(std::list<std::string> dirs);

	private:

	/* Processing Fns */

void    searchCache(PQItem *, std::list<PQItem *> &results);
void    updateCache(std::list<PQItem *> &results);
void    filterList(cert *c, std::list<PQItem *> &results);



	/* locking */
void	lockQueues()   { qMtx.lock();   }
void	unlockQueues() { qMtx.unlock(); }

	RsMutex qMtx;
	RsMutex dMtx;

	/* threading queues */
std::list<PQItem *> mIncoming;
std::list<PQItem *> mOutgoing;

std::map<std::string, std::string> sharedDirs;


void	processQueue();

/* THIS IS THE ONLY BIT WHICH DEPENDS ON PQITEM STRUCTURE */
void	loadDirectory(PQItem *dir);
void    loadRootDirs(PQFileItem *dir);

};



#endif

