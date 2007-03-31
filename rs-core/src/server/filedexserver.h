/*
 * "$Id: filedexserver.h,v 1.16 2007-03-21 18:45:41 rmf24 Exp $"
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



#ifndef MRK_PQI_FILEDEX_SERVER_HEADER
#define MRK_PQI_FILEDEX_SERVER_HEADER

/* 
 * Slightly more complete server....
 * has a filedex pointer, which manages the local indexing/searching.
 *
 */

/**************** PQI_USE_XPGP ******************/
#if defined(PQI_USE_XPGP)

#include "pqi/xpgpcert.h"

#else /* X509 Certificates */
/**************** PQI_USE_XPGP ******************/

#include "pqi/sslcert.h"

#endif /* X509 Certificates */
/**************** PQI_USE_XPGP ******************/

#include "pqi/pqi.h"
#include "pqi/pqiindic.h"
#include <map>
#include <deque>
#include <list>
#include <map>
#include <iostream>

#include "dbase/filedex.h"
#include "dbase/filelook.h"
#include "server/pqifiler.h"


#ifdef PQI_USE_CHANNELS
	#include "pqi/pqichannel.h"
	#include "pqi/p3channel.h"
#endif
	
#define MAX_RESULTS 100 // nice balance between results and traffic.


/* So we want the FILEDEX server to also contain
 * a directory struct for the more freely available
 * data that is on other comps.
 *
 * To do this it needs to store a diectory tree.
 */

class DirNode;

class DirNode
{
	public:
		
	std::string name;
	int lupdated;

	std::list<PQFileItem *> files;
	std::list<DirNode *> subdirs;
	std::ostream& print(std::ostream &out, int lvl);
};


class DirBase: public DirNode
{
	public:
	DirBase():p(NULL) {return;}
	Person *p;
	std::ostream& print(std::ostream &out);
};

class filedexserver
{
	public:
	filedexserver();

#ifdef USE_FILELOOK
int 	setFileLook(fileLook*);
#else /*************************************************/
int	setFileDex(filedex *fd);
#endif

void    loadWelcomeMsg(); /* startup message */

int	setSearchInterface(P3Interface *si, sslroot *sr);
int	additem(SearchItem *item, char *fname);

int	sendChat(std::string msg);
int	sendRecommend(PQFileItem *fi, std::string msg);
int     sendMessage(MsgItem *item);
int     checkOutgoingMessages();

int 	getChat();
/* std::deque<std::string> &getChatQueue(); */
std::list<ChatItem *> getChatQueue(); 

std::list<MsgItem *> &getMsgList();
std::list<MsgItem *> getNewMsgs();
std::map<SearchItem *, std::list<PQFileItem *> > &getResults();
std::map<SearchItem *, std::list<PQFileItem *> > getNewResults();
std::list<DirBase *>  &getDirList() { return dirlist; }
std::list<FileTransferItem *> getTransfers();


	// cleaning up....
int	removeMsgItem(int itemnum);
int	removeSearchResults(int itemnum);
	// alternative versions.
int	removeMsgItem(MsgItem *mi);
int	removeSearchResults(SearchItem *si);

// local commands.
int     cancelSearch(SearchItem *item);
int     doSearch(std::list<std::string> terms);
int 	getSearchFile(PQFileItem *item);
int 	getSearchFile(MsgItem *item);

// access to search info is also required.
void 	clear_old_transfers() { filer -> clearFailedTransfers(); }
void 	cancelTransfer(PQFileItem *i) { filer -> cancelFile(i); }

std::list<std::string> &getSearchDirectories();
int 	addSearchDirectory(std::string dir);
int 	removeSearchDirectory(std::string dir);
int 	reScanDirs();
int 	check_dBUpdate();

int	load_config();
int	save_config();

std::string	getSaveDir();
void		setSaveDir(std::string d);
void		setConfigDir(std::string d) { config_dir = d; }
bool		getSaveIncSearch();
void		setSaveIncSearch(bool v);

int	tick();
int	status();

int	handleDirectoryRequest(SearchItem *item);
int     printDirectoryListings();

	private:
int	handleSearch(SearchItem *item); /* search for locally */
PQFileItem* getLocalSearchResult(); /* returns answers to above */

int	handleFileRequest(PQFileItem *item);
int     saveSearchResult(PQFileItem *item);

int	handleDirectoryResult(PQFileItem *item);

int	handleInputQueues();
int	handleOutputQueues();

std::map<PQFileItem *, std::istream *> ofiles;
std::map<PQFileItem *, std::ostream *> ifiles;

std::list<ChatItem *> ichat;
std::list<MsgItem *> imsg;
std::list<MsgItem *> nmsg;
std::list<MsgItem *> msgOutgoing; /* ones that haven't made it out yet! */

std::map<SearchItem *, std::list<PQFileItem *> > searchResults;
std::map<SearchItem *, std::list<PQFileItem *> > nresults;

std::list<DirBase *>  dirlist;
int dirLastPrune;

std::list<std::string> dbase_dirs;


P3Interface *pqisi;
sslroot	*sslr;

#ifdef USE_FILELOOK
	fileLook *flook;
#else /***********************************************************/
	filedex *findex;
#endif

pqifiler *filer;

std::string config_dir;
std::string save_dir;
bool	save_inc; // is savedir include in share list.

// bool state flags.
	public:
	Indicator msgChanged;
	Indicator msgMajorChanged;
	Indicator chatChanged;
	Indicator searchChanged; // addition
	Indicator searchMajorChanged; // removal
	Indicator dirListChanged; // addition
	Indicator dirListMajorChanged; // removal


// for ReIndexing the dBase.
int     lastUpdate;
int     updatePeriod;


// For Passing the Dir Results through to the RsIface. 
	public:
PQFileItem *getDirPassThrough();
void   	enableDirPassThrough();

	private:
bool 	DirPassThrough;
std::list<PQFileItem *> dirPassThroughList;
 
#ifdef PQI_USE_CHANNELS

	public:
	// Channel stuff.
void	setP3Channel(p3channel *p3c);
int     getAvailableChannels(std::list<pqichannel *> &chans);
int     getChannelMsgList(channelSign s, std::list<chanMsgSummary> &summary);
channelMsg *getChannelMsg(channelSign s, MsgHash mh);

	Indicator channelsChanged; // everything!

	private:
p3channel *p3chan;

#endif

};

// Utility Functions... to split up a path into subdirs.
int     breakupDirlist(std::string path, std::list<std::string> &subdirs);

#endif // MRK_PQI_FILEDEX_SERVER_HEADER


