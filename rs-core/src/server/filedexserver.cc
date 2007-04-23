/*
 * "$Id: filedexserver.cc,v 1.23 2007-04-07 08:41:00 rmf24 Exp $"
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




/* So this is a test pqi server.....
 *
 * the idea is that this holds some
 * random data....., and responds to 
 * requests of a pqihandler.
 *
 */

// as it is a simple test...
// don't make anything hard to do.
//

#include "server/filedexserver.h"
#include <fstream>
#include <time.h>

#include "pqi/pqibin.h"
#include "pqi/pqiarchive.h"
#include "pqi/pqidebug.h"

#include <sstream>

const int fldxsrvrzone = 47659;

// This Fn is now exported..... (useful for all).
int	breakupDirlist(std::string path, std::list<std::string> &subdirs);


/* Another little hack ..... unique message Ids
 * will be handled in this class.....
 * These are unique within this run of the server, 
 * and are not stored long term....
 *
 * Only 3 entry points:
 * (1) from network....
 * (2) from local send
 * (3) from storage...
 */

static unsigned int msgUniqueId = 1;
unsigned int getNewUniqueMsgId()
{
	return msgUniqueId++;
}



filedexserver::filedexserver()
	:pqisi(NULL), sslr(NULL), 
#ifdef USE_FILELOOK
	flook(NULL),
#else /**********************************************************/
	findex(NULL), 
#endif
	filer(NULL), save_dir("."), 
	msgChanged(1), msgMajorChanged(1), chatChanged(1), 
	searchChanged(1), searchMajorChanged(1), 
	dirListChanged(1), dirListMajorChanged(1), 
	lastUpdate(0), updatePeriod(300), // 3 minutes.
        DirPassThrough(false) 
#ifdef PQI_USE_CHANNELS
	,channelsChanged(1)
#endif
{
#ifdef PQI_USE_CHANNELS
	p3chan = NULL;
#endif

}

int	filedexserver::setSearchInterface(P3Interface *si, sslroot *sr)
{
	pqisi = si;
	sslr = sr;
	return 1;
}

#ifdef USE_FILELOOK

int     filedexserver::setFileLook(fileLook *fl)
{
	flook = fl;
	filer = new pqifiler(fl);
	return 1;
}

#else /****************************************************************/

int     filedexserver::setFileDex(filedex *fd)
{
	findex = fd;
	filer = new pqifiler(fd);
	return 1;
}

#endif



std::list<FileTransferItem *> filedexserver::getTransfers()
{
	return filer->getStatus();
}



// This function needs to be divided up.
int     filedexserver::handleInputQueues()
{
	// get all the incoming results.. and print to the screen.
	SearchItem *si;
	PQFileItem *fi;
	PQFileItem *pfi;
	// Loop through Search Results.
	int i = 0;
	int i_init = 0;
	while((fi = pqisi -> GetSearchResult()) != NULL)
	{
		std::ostringstream out;
		if (i++ == i_init)
		{
			out << "Recieved Search Results:" << std::endl;
		}
		fi -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());

		saveSearchResult(fi);
	}

	// now requested Searches.
	i_init = i;
	while((si = pqisi -> RequestedSearch()) != NULL)
	{
		std::ostringstream out;
		if (i++ == i_init)
		{
			out << "Requested Search:" << std::endl;
		}
		si -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		handleSearch(si);
	}

	// now Cancelled Searches.
	i_init = i;
	while((si = pqisi -> CancelledSearch()) != NULL)
	{
		std::ostringstream out;
		if (i++ == i_init)
		{
			out << "Cancelled Search:" << std::endl;
		}
		si -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		delete si;
	}

	// now File Input.
	i_init = i;
	while((pfi = pqisi -> GetFileItem()) != NULL )
	{
		std::ostringstream out;
		if (i++ == i_init)
		{
			out << "Incoming(Net) File Item:" << std::endl;
		}
		pfi -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		filer -> recvPQFileItem(pfi);
	}

	if (i > 0)
	{
		return 1;
	}
	return 0;
}


// This function needs to be divided up.
int     filedexserver::handleOutputQueues()
{
	// get all the incoming results.. and print to the screen.
	PQItem *fi;
	// Loop through Filer stuff.
	int i = 0;
	while((fi = filer -> sendPQFileItem()) != NULL)
	{
		std::ostringstream out;
		if (i++ == 0)
		{
			out << "Outgoing filer -> PQFileItem:" << std::endl;
		}
		fi -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		pqisi -> SendFileItem((PQFileItem *) fi);
	}

	// Loop through Local Search Results.
	while((fi = getLocalSearchResult()) != NULL)
	{
		std::ostringstream out;
		if (i++ == 0)
		{
			out << "Outgoing Local Search Result -> PQFileItem:" << std::endl;
		}
		fi -> print(out);
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		pqisi -> SendFileItem((PQFileItem *) fi);
	}

	if (i > 0)
	{
		return 1;
	}
	return 0;
}






int	filedexserver::tick()
{
	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::tick()");

	if (pqisi == NULL)
	{
		std::ostringstream out;
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
			"filedexserver::tick() Invalid Interface()");

		return 1;
	}

	int moreToTick = 0;

	if (0 < pqisi -> tick())
	{
		moreToTick = 1;
	}

	if (0 < filer -> tick())
	{
		moreToTick = 1;
	}


	if (0 < handleInputQueues())
	{
		moreToTick = 1;
	}

	if (0 < handleOutputQueues())
	{
		moreToTick = 1;
	}

	if (0 < getChat())
	{
		moreToTick = 1;
	}
	checkOutgoingMessages(); /* don't worry about increasing tick rate! */

	return moreToTick;
}


int	filedexserver::status()
{
	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::status()");

	if (pqisi == NULL)
	{
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
			"filedexserver::status() Invalid Interface()");
		return 1;
	}

	pqisi -> status();

	return 1;
}

#ifdef USE_FILELOOK

int	filedexserver::handleSearch(SearchItem *item)
{

	// break up string into terms.

	if (item -> datatype == PQI_SI_DATATYPE_DIR)
	{
		/* Mismatch! */
		PQFileItem *fi = new PQFileItem();
		fi -> copyIDs(item);
		fi -> path = item -> data;

		flook -> lookUpDirectory(fi);
		delete item;
		//delete fi; // no, handled by flook.
	}
	else
	{
		std::cerr << "filedexserver::Search Not Implemented Yet!" << std::endl;
		//std::list<std::string> terms = createtermlist(item -> data);
		//flook -> search(terms, MAX_RESULTS, false);
	}

	return 1;
}

PQFileItem *filedexserver::getLocalSearchResult()
{
	return (PQFileItem *) flook -> getResult();
}

#else /******************************************************************/

PQFileItem *filedexserver::getLocalSearchResult()
{
	return NULL;
}

int	filedexserver::handleSearch(SearchItem *item)
{
	// search through the dbase.

	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::handleSearch()");

	// convert to format for filedex search....
	// hacky conversion.

	std::list<std::string> terms;
	std::list<fdex *> results;
	std::list<fdex *>::iterator it;

	// for the moment will use the strings from.

	// break up string into terms.
	terms = createtermlist(item -> data);

	if (findex == NULL)
	{
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
			"Warning No Dbase attached!");
		delete item;
		return -1;
	}

	check_dBUpdate();
	if (item -> datatype == PQI_SI_DATATYPE_DIR)
	{
		results = findex -> dirlisting(item -> data);
	}
	else
	{
		results = findex -> search(terms, MAX_RESULTS, false);
	}

	// write back each reply.
	for(it = results.begin(); it != results.end(); it++)
	{
		// reply.
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone,
			"Found Match Sending Reply!");

		PQFileItem *fi = new PQFileItem();

		// not using copy search item.....
		// instead just put the file name into title....

		fi -> name = (*it) -> file;
		fi -> path = (*it) -> subdir;
		fi -> hash = "N/A";
		fi -> ext = (*it) -> ext;
		fi -> size = (*it) -> len;
		fi -> copyIDs(item);
		if (item -> datatype == PQI_SI_DATATYPE_DIR)
		{
			if ((fi -> size == 0) && (fi -> ext == "DIR"))
			{
				fi -> reqtype = PQIFILE_DIRLIST_DIR;
			}
			else
			{
				fi -> reqtype = PQIFILE_DIRLIST_FILE;
			}
		}

		{
		  std::ostringstream out;
		  fi -> print(out);
		  pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		}

		pqisi -> SendSearchResult(fi);
	}
	delete item;
	return 1;
}

#endif




std::string	filedexserver::getSaveDir()
{
	return save_dir;
}

void	filedexserver::setSaveDir(std::string d)
{
	save_dir = d;
	filer -> setSavePath(save_dir);
}


bool 	filedexserver::getSaveIncSearch()
{
	return save_inc;
}

void	filedexserver::setSaveIncSearch(bool v)
{
	save_inc = v;
}


// must not delete the item.
int	filedexserver::getSearchFile(PQFileItem *item)
{
	// send to filer.
	filer -> getFile(item->clone());
	return 1;
}


// must not delete the item.
int	filedexserver::getSearchFile(MsgItem *item)
{
	std::list<MsgFileItem>::iterator it;
	for(it = item -> files.begin(); it != item -> files.end(); it++)
	{
		PQFileItem *fi = new PQFileItem();

		fi -> copyIDs(item);
		fi -> sid = getPQIsearchId();
		fi -> subtype = PQI_FI_SUBTYPE_REQUEST;
		fi -> name = it -> name;
		fi -> hash = it -> hash;
		fi -> size = it -> size;

		// getfile.
		filer->getFile(fi);
	}

	return 1;
}


int	filedexserver::saveSearchResult(PQFileItem *item)
{
	// loop through the searchResults
	// find matching sid, 
	// add the that list.
	// save in array
	//
	// first if flag set... send to directory results.
	if (item -> reqtype != PQIFILE_STD)
	{
		return handleDirectoryResult(item);
	}

	std::map<SearchItem *, std::list<PQFileItem *> >::iterator it;
	for(it = searchResults.begin(); it != searchResults.end(); it++)
	{
		if ((it -> first) -> sid == item -> sid)
		{
			// correct search.
			(it -> second).push_back(item);

			// add into new queue.
			std::list<PQFileItem *> &rl = nresults[it -> first];
			rl.push_back(item);

			// signal changed.
			searchChanged.IndicateChanged();
			return 1;
		}
	}


	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::saveSearchResult() Unable to Match result to srch req");

	return 0;
}


int	filedexserver::doSearch(std::list<std::string> terms)
{
	// construct search item....
	//
	// send on merry way...
	//
	// archive it.
	//
	// copy the search result.

	std::list<std::string>::iterator it;

	SearchItem *sitem = new SearchItem();

	for(it = terms.begin(); it != terms.end();) 
	{
		sitem -> data += (*it);
		it++;
		if (it != terms.end())
		{
			sitem -> data += " ";
		}
	}

	sitem -> sid = getPQIsearchId();

	std::list<PQFileItem *> efil; // empty list.

	// save in map
	searchResults[sitem] = efil;
	nresults[sitem] = efil;
	searchChanged.IndicateChanged();

	pqisi -> Search((SearchItem *) sitem -> clone());
	return 1;
}

int	filedexserver::cancelSearch(SearchItem *item)
{
	// delete matching search request + results
	// from memory.

	return 1;
}

/***************** Chat Stuff **********************/

int     filedexserver::sendChat(std::string msg)
{
	// make chat item....
	ChatItem *ci = new ChatItem();
	ci -> sid = getPQIsearchId();
	ci -> p = sslr -> getOwnCert();

	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::sendChat()");

	ci -> msg = msg;

	/* will come back to us anyway (for now) */
	//ichat.push_back(ci -> clone());
	//chatChanged.IndicateChanged();

	{
	  std::ostringstream out;
	  out << "Chat Item we are sending:" << std::endl;
	  ci -> print(out);
	  pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
	}

	/* to global .... */
	pqisi -> SendGlobalMsg(ci);
	return 1;
}

int 	filedexserver::getChat()
{
	ChatItem *ci;
	int i = 0;
	while((ci = pqisi -> GetMsg()) != NULL)
	{
		++i;
		ci -> epoch = time(NULL);
		std::string mesg;

		if (ci -> subtype == PQI_MI_SUBTYPE_MSG)
		{
			// then a message..... push_back.
			MsgItem *mi = (MsgItem *) ci;
			if (mi -> p == sslr->getOwnCert())
			{
				/* from the loopback device */
				mi -> msgflags = PQI_MI_FLAGS_OUTGOING;
			}
			else
			{
				/* from a peer */
				mi -> msgflags = 0;
			}

			/* new as well! */
			mi -> msgflags |= PQI_MI_FLAGS_NEW;
			/* STORE MsgID */
			mi -> sid = getNewUniqueMsgId();

			imsg.push_back(mi);
			//nmsg.push_back(mi);
			msgChanged.IndicateChanged();
		}
		else
		{
			// push out the chat instead.
			ichat.push_back(ci);
			chatChanged.IndicateChanged();
		}
	}

	if (i > 0)
	{
		return 1;
	}
	return 0;
}
	

std::list<ChatItem *> filedexserver::getChatQueue()
{
	std::list<ChatItem *> ilist = ichat;
	ichat.clear();
	return ilist;
}


std::list<MsgItem *> &filedexserver::getMsgList()
{
	return imsg;
}

std::list<MsgItem *> &filedexserver::getMsgOutList()
{
	return msgOutgoing;
}



std::list<MsgItem *> filedexserver::getNewMsgs()
{
	std::list<MsgItem *> tmplist = nmsg;
	nmsg.clear();
	return tmplist;
}

std::map<SearchItem *, std::list<PQFileItem *> > &filedexserver::getResults()
{
	return searchResults;
}

std::map<SearchItem *, std::list<PQFileItem *> > filedexserver::getNewResults()
{
	std::map<SearchItem *, std::list<PQFileItem *> > res;
	res = nresults;
	nresults.clear();
	return res;
}

	
int     filedexserver::removeSearchResults(int itemnum)
{
	std::map<SearchItem *, std::list<PQFileItem *> >::iterator it;
	std::list<PQFileItem *>::iterator rit;

	int i;
	for(i = 1, it = searchResults.begin(); (it != searchResults.end()); it++, i++)
	{
		if (i == itemnum)
		{
			// clean up the list.
			rit = (it -> second).begin();
			while(rit != (it -> second).end())
			{
				PQFileItem *fi = (*rit);
				delete fi;
				rit = (it -> second).erase(rit);
			}

			delete (it -> first);
			searchResults.erase(it);

			searchChanged.IndicateChanged();
			searchMajorChanged.IndicateChanged();
			return 1;
		}
	}
	return 0;
}

int     filedexserver::removeSearchResults(SearchItem *si)
{
	std::map<SearchItem *, std::list<PQFileItem *> >::iterator it;
	std::list<PQFileItem *>::iterator rit;

	// find place.
	for(it = searchResults.begin(); (si != it -> first) && 
			(it != searchResults.end()); it++);
	if (it != searchResults.end())
	{
		// clean up the list.
		rit = (it -> second).begin();
		while(rit != (it -> second).end())
		{
			PQFileItem *fi = (*rit);
			delete fi;
			rit = (it -> second).erase(rit);
		}
		searchResults.erase(it);
		delete(si);

		searchChanged.IndicateChanged();
		searchMajorChanged.IndicateChanged();
		return 1;
	}
	return 0;
}

/* remove based on the unique mid (stored in sid) */
int     filedexserver::removeMsgId(unsigned long mid)
{
	std::list<MsgItem *>::iterator it;
	int i;

	for(it = imsg.begin(); it != imsg.end(); it++)
	{
		if ((*it)->sid == mid)
		{
			MsgItem *mi = (*it);
			imsg.erase(it);
			delete mi;
			msgChanged.IndicateChanged();
			msgMajorChanged.IndicateChanged();
			return 1;
		}
	}

	/* try with outgoing messages otherwise */
	for(it = msgOutgoing.begin(); it != msgOutgoing.end(); it++)
	{
		if ((*it)->sid == mid)
		{
			MsgItem *mi = (*it);
			msgOutgoing.erase(it);
			delete mi;
			msgChanged.IndicateChanged();
			msgMajorChanged.IndicateChanged();
			return 1;
		}
	}

	return 0;
}

int     filedexserver::markMsgIdRead(unsigned long mid)
{
	std::list<MsgItem *>::iterator it;
	int i;

	for(it = imsg.begin(); it != imsg.end(); it++)
	{
		if ((*it)->sid == mid)
		{
			MsgItem *mi = (*it);
			mi -> msgflags &= ~(PQI_MI_FLAGS_NEW);
			msgChanged.IndicateChanged();
			return 1;
		}
	}
	return 0;
}

int     filedexserver::removeMsgItem(int itemnum)
{
	std::list<MsgItem *>::iterator it;
	int i;

	for(i = 1, it = imsg.begin(); (i != itemnum) && (it != imsg.end()); it++, i++);
	if (it != imsg.end())
	{
		MsgItem *mi = (*it);
		imsg.erase(it);
		delete mi;
		msgChanged.IndicateChanged();
		msgMajorChanged.IndicateChanged();
		return 1;
	}
	return 0;
}


int     filedexserver::removeMsgItem(MsgItem *mi)
{
	std::list<MsgItem *>::iterator it;
	for(it = imsg.begin(); (mi != *it) && (it != imsg.end()); it++);
	if (it != imsg.end())
	{
		imsg.erase(it);
		delete mi;
		msgChanged.IndicateChanged();
		msgMajorChanged.IndicateChanged();
		return 1;
	}
	return 0;
}

/* This is the old fltkgui send recommend....
 * can only handle one single file....
 * will maintain this fn.... but it is deprecated.
 */

int     filedexserver::sendRecommend(PQFileItem *fi, std::string msg)
{
	// make chat item....
	MsgItem *mi = new MsgItem();
	mi -> sid = getPQIsearchId();

	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::sendRecommend()");

	mi -> msg = msg;

	if (fi != NULL)
	{
		MsgFileItem mfi;
		mfi.name = fi -> name;
		mfi.hash = fi -> hash;
		mfi.size = fi -> size;

		mi -> files.push_back(mfi);
	}
	else
	{
		/* nuffink */
	}

	pqisi -> SendMsg(mi);
	return 1;
}


int     filedexserver::sendMessage(MsgItem *item)
{
	item -> sid = getPQIsearchId();

	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"filedexserver::sendMessage()");

	if (item -> p)
	{
		/* add pending flag */
		item->msgflags |= 
			(PQI_MI_FLAGS_OUTGOING | 
			 PQI_MI_FLAGS_PENDING);
		/* STORE MsgID */
		item -> sid = getNewUniqueMsgId();
		msgOutgoing.push_back(item);
	}
	else
	{
		delete item;
	}
	return 1;
}

int     filedexserver::checkOutgoingMessages()
{
	/* iterate through the outgoing queue 
	 *
	 * if online, send
	 */

	std::list<MsgItem *>::iterator it;
	for(it = msgOutgoing.begin(); it != msgOutgoing.end();)
	{
	 	/* if online, send it */
		if (((*it) -> p -> Status() & PERSON_STATUS_CONNECTED) 
		    || ((*it) -> p == sslr->getOwnCert()))
		{
			/* send msg */
			pqioutput(PQL_ALERT, fldxsrvrzone, 
				"filedexserver::checkOutGoingMessages() Sending out message");
			/* remove the pending flag */
			(*it)->msgflags &= ~PQI_MI_FLAGS_PENDING;

			pqisi -> SendMsg(*it);
			it = msgOutgoing.erase(it);
		}
		else
		{
			pqioutput(PQL_ALERT, fldxsrvrzone, 
				"filedexserver::checkOutGoingMessages() Delaying until available...");
			it++;
		}
	}
	return 0;
}


int     filedexserver::addSearchDirectory(std::string dir)
{
	dbase_dirs.push_back(dir);
	reScanDirs();
	return 1;
}

int     filedexserver::removeSearchDirectory(std::string dir)
{
	std::list<std::string>::iterator it;
	for(it = dbase_dirs.begin(); (it != dbase_dirs.end()) 
			&& (dir != *it); it++);
	if (it != dbase_dirs.end())
	{
		dbase_dirs.erase(it);
	}

	reScanDirs();
	return 1;
}

std::list<std::string> &filedexserver::getSearchDirectories()
{
	return dbase_dirs;
}

int	filedexserver::check_dBUpdate()
{
	if (lastUpdate + updatePeriod < time(NULL))
	{
		return reScanDirs();
	}
	return 0;
}
	

int     filedexserver::reScanDirs()
{
#ifdef USE_FILELOOK

	flook -> setSharedDirectories(dbase_dirs);

#else
	findex -> clear();
	std::list<DirItem> dbase_list;
	std::list<std::string>::iterator it;

	for(it = dbase_dirs.begin(); it != dbase_dirs.end(); it++)
	{
		dbase_list.push_back(DirItem(*it, "", 2));
	}
	findex -> load(dbase_list);
#endif
	lastUpdate = time(NULL);

	return 1;
}

static const std::string fdex_dir("FDEX_DIR");
static const std::string save_dir_ss("SAVE_DIR");
static const std::string save_inc_ss("SAVE_INC");

int     filedexserver::save_config()
{
	std::list<std::string>::iterator it;
	std::string empty("");

	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, 
		"fildexserver::save_config()");

	sslr -> setSetting(save_dir_ss, getSaveDir());
	if (getSaveIncSearch())
	{
		sslr -> setSetting(save_inc_ss, "true");
	}
	else
	{
		sslr -> setSetting(save_inc_ss, "false");
	}

	int i;
	for(it = dbase_dirs.begin(), i = 0; (it != dbase_dirs.end()) 
		&& (i < 1000); it++, i++)
	{
		std::string name = fdex_dir;
		int d1, d2, d3;
		d1 = i / 100;
		d2 = (i - d1 * 100) / 10;
		d3 = i - d1 * 100 - d2 * 10;

		name += '0'+d1;
		name += '0'+d2;
		name += '0'+d3;

		sslr -> setSetting(name, (*it));
	}
	// blank other ones.
	bool done = false;
	for(; (i < 1000) && (!done); i++)
	{
		std::string name = fdex_dir;
		int d1, d2, d3;
		d1 = i / 100;
		d2 = (i - d1 * 100) / 10;
		d3 = i - d1 * 100 - d2 * 10;

		name += '0'+d1;
		name += '0'+d2;
		name += '0'+d3;

		if (empty == sslr -> getSetting(name))
		{
			done = true;
		}
		else
		{
			sslr -> setSetting(name, empty);

			std::ostringstream out;
			out << "Blanking Setting(" << name;
			out << ")" << std::endl;
			pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
		}
	}



	/* now we create a pqiarchive, and stream all the msgs/fts
	 * into it
	 */

	std::string statelog = config_dir + "/state.rst";

	BinFileInterface *out = new BinFileInterface((char *) statelog.c_str(), BIN_FLAGS_WRITEABLE);
        pqiarchive *pa_out = new pqiarchive(out, BIN_FLAGS_WRITEABLE, sslr);
	bool written = false;

	std::list<MsgItem *>::iterator mit;
	for(mit = imsg.begin(); mit != imsg.end(); mit++)
	{
		MsgItem *mi = (*mit)->clone();
		if (pa_out -> SendItem(mi))
		{
			written = true;
		}
		
	}

	for(mit = msgOutgoing.begin(); mit != msgOutgoing.end(); mit++)
	{
		MsgItem *mi = (*mit)->clone();
		mi -> msgflags |= PQI_MI_FLAGS_PENDING;
		if (pa_out -> SendItem(mi))
		{
			written = true;
		}
		
	}

	std::list<FileTransferItem *>::iterator fit;
	std::list<FileTransferItem *> ftlist = filer -> getStatus();
	for(fit = ftlist.begin(); fit != ftlist.end(); fit++)
	{
		if (pa_out -> SendItem(*fit))
		{
			written = true;
		}
	}

	if (!written)
	{
		/* need to push something out to overwrite old data! (For WINDOWS ONLY) */
	}

	delete pa_out;	
	return 1;
}

int     filedexserver::load_config()
{
	std::list<std::string>::iterator it;

	int i;
	std::string empty("");
	std::string dir("notempty");
	std::string str_true("true");

	std::string sdir = sslr -> getSetting(save_dir_ss);
	if (sdir != empty)
	{
		setSaveDir(sdir);
	}

	std::string sinc = sslr -> getSetting(save_inc_ss);
	if (sdir != empty)
	{
		setSaveIncSearch(sdir == str_true);
	}

	dbase_dirs.clear();

	for(i = 0; (i < 1000) && (dir != empty); i++)
	{
		std::string name = fdex_dir;
		int d1, d2, d3;
		d1 = i / 100;
		d2 = (i - d1 * 100) / 10;
		d3 = i - d1 * 100 - d2 * 10;

		name += '0'+d1;
		name += '0'+d2;
		name += '0'+d3;

		dir = sslr -> getSetting(name);
		if (dir != empty)
		{
			dbase_dirs.push_back(dir);
		}
	}
	if (dbase_dirs.size() > 0)
	{
		std::ostringstream out;
		out << "Loading " << dbase_dirs.size();
		out << " Directories" << std::endl;
		pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());

		reScanDirs();
	}

	/* load msg/ft */
	std::string statelog = config_dir + "/state.rst";

	// XXX Fix Interface!
	BinFileInterface *in = new BinFileInterface((char *) statelog.c_str(), BIN_FLAGS_READABLE);
        pqiarchive *pa_in = new pqiarchive(in, BIN_FLAGS_READABLE, sslr);
	PQItem *item;
	MsgItem *mitem;
	PQFileItem *fitem;

	while((item = pa_in -> GetItem()))
	{
		switch(item->type)
		{
			case PQI_ITEM_TYPE_FILEITEM:
				/* add to ft queue */
				if (NULL != (fitem = dynamic_cast<PQFileItem *>(item)))
				{
					filer -> getFile(fitem);
				}
				else
				{
					delete item;
				}
				break;
			case PQI_ITEM_TYPE_CHATITEM:
				if (NULL != (mitem = dynamic_cast<MsgItem *>(item)))
				{
					/* switch depending on the PENDING 
					 * flags
					 */
					/* STORE MsgID */
					mitem->sid = getNewUniqueMsgId();
					if (mitem -> msgflags & PQI_MI_FLAGS_PENDING)
					{
						std::cerr << "MSG_PENDING";
						std::cerr << std::endl;
						mitem->print(std::cerr);
						msgOutgoing.push_back(mitem);
					}
					else
					{
						imsg.push_back(mitem);
					}
				}
				else
				{
					delete item;
				}
				/* add to chat queue */
				break;
			default:
				/* unexpected */
				break;
		}
	}

	delete pa_in;	

	return 1;
}


int     filedexserver::handleDirectoryRequest(SearchItem *item)
{
	// just send it off -> don't bother tracking... at the mo
	std::ostringstream out;
	out << "filedexserver::handleDirectoryRequest() for: " << std::endl;
	item -> print(out);
	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());

	pqisi -> SearchSpecific(item);
	return 1;
}




PQFileItem *filedexserver::getDirPassThrough()
{
	PQFileItem *item = NULL;
	if (dirPassThroughList.size() > 0)
	{
		item = dirPassThroughList.front();
		dirPassThroughList.pop_front();
	}
	return item;
} 

void 	filedexserver::enableDirPassThrough()
{
	DirPassThrough = true;
}

/* This Function now has two behaviours....
 * depending on the setting of the  variable DirPassThrough (def = false)
 *
 * XXX -> note directory entries aren't deleted???? XXX TOFIX!
 */

int     filedexserver::handleDirectoryResult(PQFileItem *item)
{
	// find the head of the tree.	
	std::list<std::string> subdirs;
	std::list<std::string>::iterator sit;
	std::list<DirBase *>::iterator it;
	std::list<DirNode *>::iterator nit;
	std::list<PQFileItem *>::iterator fit;

	DirBase *base;
	DirNode *node, *nnode;

	if (DirPassThrough)
	{
		/* 
		 */
		dirPassThroughList.push_back(item->clone());
		dirListChanged.IndicateChanged();
		return 1;
	}

	for(it = dirlist.begin();(it != dirlist.end()) && ((*it)->p!=item->p); it++);

	if (it == dirlist.end())
	{
		base = new DirBase();
		base -> p = item -> p;
		base -> lupdated = time(NULL);
		dirlist.push_back(base);
		dirListChanged.IndicateChanged();
	}
	else
	{
		base = *it;
		base -> lupdated = time(NULL);
	}

	breakupDirlist(item -> path, subdirs);

	node = base;

	for(sit = subdirs.begin(); sit != subdirs.end(); sit++)
	{
		for(nit = node->subdirs.begin();
			(nit != node->subdirs.end()) && 
			((*nit)->name != *sit); nit++);

		if (nit == node->subdirs.end())
		{
			// add new subnode.
			nnode = new DirNode();
			nnode -> name = *sit;
			nnode -> lupdated = time(NULL);
			node -> subdirs.push_back(nnode);
			node = nnode;
			dirListChanged.IndicateChanged();
		}
		else
		{
			node = *nit;
		}
	}

	if (item -> reqtype == PQIFILE_DIRLIST_FILE)
	{
		// check if it exists
		for(fit = node->files.begin();
			(fit != node->files.end()) && 
			((*fit)->name != item -> name); fit++);

		if (fit == node->files.end())
		{
			node->files.push_back(item);
			dirListChanged.IndicateChanged();
		}
	}
	else
	{
		// check if it exists
		for(nit = node->subdirs.begin();
			(nit != node->subdirs.end()) && 
			((*nit)->name != item -> name); nit++);

		if (nit == node->subdirs.end())
		{
			nnode = new DirNode();
			nnode -> name = item -> name;
			nnode -> lupdated = time(NULL);
			node -> subdirs.push_back(nnode);

			dirListChanged.IndicateChanged();
			
		}
	}

	printDirectoryListings();
	return 1;
}

int	breakupDirlist(std::string path, std::list<std::string> &subdirs)
{
	int start = 0;
	unsigned int i;
	for(i = 0; i < path.length(); i++)
	{
		if (path[i] == '/')
		{
			if (i - start > 1)
			{
				subdirs.push_back(path.substr(start, i-start));
			}
			start = i+1;
		}
	}
	// get the final one.
	if (i - start > 1)
	{
		subdirs.push_back(path.substr(start, i-start));
	}
	return 1;
}


std::ostream& DirBase::print(std::ostream &out)
{
	if (p == NULL)
	{
		out << "DirList for: UNKNOWN" << std::endl;
	}
	else
	{
		out << "DirList for: " << p -> Name() << std::endl;
	}
	std::list<DirNode *>::iterator nit;
	std::list<PQFileItem *>::iterator fit;
	for(nit = subdirs.begin(); nit != subdirs.end(); nit++)
		(*nit) -> print(out, 1);
	for(fit = files.begin(); fit != files.end(); fit++)
	{
		out << "File " << (*fit) -> name << std::endl;
	}
	return out;
}


std::ostream& DirNode::print(std::ostream &out, int lvl)
{
	std::list<DirNode *>::iterator nit;
	std::list<PQFileItem *>::iterator fit;
	int i;
	for(i = 0; i < lvl-1; i++)
	{
		out << " ";
	}
	out << "-";
	out << "Dir: " << name << std::endl;
	for(nit = subdirs.begin(); nit != subdirs.end(); nit++)
		(*nit) -> print(out, lvl+1);
	for(fit = files.begin(); fit != files.end(); fit++)
	{
		for(i = 0; i < lvl; i++)
		{
			out << " ";
		}
		out << "-> File " << (*fit) -> name << std::endl;
	}
	return out;
}



int     filedexserver::printDirectoryListings()
{
	std::list<DirBase *>::iterator it;
	std::ostringstream out;
	for(it = dirlist.begin();it != dirlist.end(); it++)
	{
		(*it) -> print(out);
		out << std::endl;
	}
	pqioutput(PQL_DEBUG_BASIC, fldxsrvrzone, out.str());
	return 1;
}


#ifdef PQI_USE_CHANNELS

        // Channel stuff.
void    filedexserver::setP3Channel(p3channel *p3c)
{
	p3chan = p3c;
	return;
}

int 	filedexserver::getAvailableChannels(std::list<pqichannel *> &chans)
{
	if (!p3chan)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getAvailableChannels() p3chan == NULL");
		return 0;
	}
	return p3chan->getChannelList(chans);
}


int 	filedexserver::getChannelMsgList(channelSign s, std::list<chanMsgSummary> &summary)
{
	if (!p3chan)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getChannelMsgList() p3chan == NULL");
		return 0;
	}
	pqichannel *chan = p3chan -> findChannel(s);
	if (!chan)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getChannelMsgList() Channel don't exist!");
		return 0;
	}
	return chan -> getMsgSummary(summary);
}


channelMsg *filedexserver::getChannelMsg(channelSign s, MsgHash mh)
{
	if (!p3chan)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getChannelMsg() p3chan == NULL");
		return NULL;
	}
	pqichannel *chan = p3chan -> findChannel(s);
	if (!chan)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getChannelMsg() Channel don't exist!");
		return NULL;
	}
	channelMsg *msg = chan -> findMsg(mh);
	if (!msg)
	{
		pqioutput(PQL_ALERT, fldxsrvrzone, 
			"fildexserver::getChannelMsg() Msg don't exist!");
		return NULL;
	}
	return msg;
}


	
#endif


void filedexserver::loadWelcomeMsg()
{
	/* Load Welcome Message */
	MsgItem *msg = new MsgItem();

	msg -> p = getSSLRoot() -> getOwnCert();

	msg -> title  = "Welcome to Retroshare";
	msg -> header = "Basic Instructions";
	msg -> sendTime = 0;

	msg -> msg    = "Send and receive messages\n"; 
	msg -> msg   += "with your friends...\n\n";

	msg -> msg   += "These can hold recommendations\n";
	msg -> msg   += "from your local shared files\n\n";

	msg -> msg   += "Add recommendations through\n";
	msg -> msg   += "the Local Files Dialog\n\n";

	msg -> msg   += "Enjoy.\n";

	imsg.push_back(msg);	
}


