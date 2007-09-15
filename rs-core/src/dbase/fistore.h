/*
 * RetroShare FileCache Module: fistore.h
 *   
 * Copyright 2004-2007 by Robert Fernie.
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

#ifndef MRK_FILE_INDEX_STORE_H
#define MRK_FILE_INDEX_STORE_H


/**********
 * Stores the FileCaches of the Peers
 * must implement 'loadCache' to 
 *
 * This class is also accessed by the GUI....
 * and the FileTransfer class.
 *
 */

#include "dbase/findex.h"
#include "dbase/cachestrapper.h"
#include "rsiface/rsiface.h"

class FileStoreResult
{
	public:
	std::string id;
	std::string path;
	std::string hash;
	std::string name;
};

class NotifyCallback
{
        public:
        NotifyCallback() 	{ return; }
virtual ~NotifyCallback() 	{ return; }
virtual void AboutToModify()	{ return; }
virtual void ModCompleted()     { return; }
};



class FileIndexStore: public CacheStore
{
	public:

	FileIndexStore(CacheTransfer *cft, NotifyBase *cb_in, 
				std::string cachedir);
virtual ~FileIndexStore();

	/* virtual functions overloaded by cache implementor */
virtual int loadCache(const CacheData &data);	  /* actual load, once data available */

	/* Search Interface - For FileTransfer Lookup */
	int searchHash(std::string hash, std::list<FileStoreResult> &results);

	/* Search Interface - For Search Interface */
	int SearchKeywords(std::list<std::string> terms, std::list<FileDetail> &results);

	/* Search Interface - For Directory Access */
	int RequestDirDetails(std::string uid, std::string path, DirDetails &details);
	int RequestDirDetails(void *ref, DirDetails &details);

	private:
	int AboutToModify();
	int ModCompleted();

	std::map<RsPeerId, FileIndex *> indices;
	NotifyBase *cb;
};


#endif
