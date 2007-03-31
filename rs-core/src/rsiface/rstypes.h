#ifndef RS_TYPES_GUI_INTERFACE_H
#define RS_TYPES_GUI_INTERFACE_H

/*
 * "$Id: rstypes.h,v 1.5 2007-03-21 18:45:41 rmf24 Exp $"
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


#include <list>
#include <iostream>
#include <string>

#define RSCERTIDLEN 16

class RsCertId
{
        public:
	RsCertId();
	RsCertId(std::string idstr);

        bool    operator<(const RsCertId &ref) const;
        bool    operator==(const RsCertId &ref) const;
        bool    operator!=(const RsCertId &ref) const;
        char    data[RSCERTIDLEN];
};

std::ostream &operator<<(std::ostream &out, const RsCertId &id);

/* use RsCertId, (not unsigned long) because the definition will change 
typedef unsigned long RsCertId; 
*/

typedef RsCertId      RsChanId; 
typedef RsCertId      RsMsgId; 

typedef std::string   RsAuthId; 


/* forward declarations of the classes */

#define INFO_SAME 0x01
#define INFO_CHG  0x02
#define INFO_NEW  0x04
#define INFO_DEL  0x08

class BaseInfo
{
	public:
	BaseInfo() :flags(0), mId(0) { return; }
	RsCertId id; /* key for matching everything */
	int flags; /* INFO_TAG above */

	/* allow this to be tweaked by the GUI Model */
	mutable unsigned int mId; /* (GUI) Model Id -> unique number */
};

class NeighbourInfo: public BaseInfo
{
	public:
	std::string name;
	std::string org;
	std::string loc;
	std::string state;
	std::string country;
	int         trustLvl;
	std::string trustString;
	std::list<RsCertId> signers;
	std::string authCode;
	int status;

	std::string acceptString;
	std::string statusString;
	std::string connectString;
	std::string lastConnect;
	std::string peerAddress;

	/* server settings */
	std::string		localAddr;
	int			localPort;
	std::string		extAddr;
	int			extPort;
	std::string		extName;

	bool			firewalled;
	bool			forwardPort;

	int			maxRate;     /* kb */

	bool ownsign;

	/* Flags to indicate if they are in
	 * chat or msg list 
	 */

	bool inChat;
	bool inMsg;
};
	
/********************** For the Directory Listing *****************/

class FileInfo: public BaseInfo
{
	public:

static const int kRsFiStatusNone = 0;
static const int kRsFiStatusStall = 1;
static const int kRsFiStatusProgress = 2;
static const int kRsFiStatusDone = 2;

	/* FileInfo(); */

	int searchId;      /* 0 if none */
	std::string path;
	std::string fname;
	std::string hash;
	std::string ext;

	int size; 
	int avail; /* how much we have */
	int status;

	bool inRecommend;

	double rank;
	int age;
};

class DirInfo: public BaseInfo
{
	public:

	DirInfo() :infoAge(0), nofiles(0), nobytes(0) { return; }
	std::string path;
	std::string dirname;
	std::list<DirInfo> subdirs;
	std::list<FileInfo> files;
	int infoAge; 
	int nofiles;
	int nobytes;

	double rank;
	int    age;

int 	 merge(const DirInfo &udir);

bool     exists(const DirInfo&);
DirInfo* existsPv(const DirInfo&);
bool     add(const DirInfo&);
int 	 update(const DirInfo &udir);


bool     exists(const FileInfo&);
FileInfo* existsPv(const FileInfo&);
bool     add(const FileInfo&);

};

class PersonInfo: public BaseInfo
{
	public:
	std::string name;
	bool online;
	int infoAge; /* time() at when this was last updated */

	DirInfo rootdir;
};

/********************** For Messages and Channels *****************/

class FileTransferInfo: public FileInfo
{
	public:
	std::string source;
	int transfered;
	double tfRate; /* kbytes */
	bool download;
	int  downloadStatus; /* 0 = Err, 1 = Ok, 2 = Done */
};


/********************** For Messages and Channels *****************/

class MessageInfo: public BaseInfo
{
	public:
	MessageInfo() {}
	RsMsgId msgId;

	std::string srcname;
	std::string title;
	std::string header;
	std::string msg;
	std::list<FileInfo> files;
	int size;  /* total of files */
	int count; /* file count     */

	int ts;
};

class ChannelInfo: public BaseInfo
{
	public:
	ChannelInfo() :publisher(false) {}
	RsChanId chanId;
	bool publisher;
	std::string chanName;
	std::list<MessageInfo> msglist;

	/* details */
	int mode;
	float rank;

	bool inBroadcast; 
	bool inSubscribe;

	int size;  /* total of msgs */
	int count; /* msg count     */
};

class ChatInfo: public BaseInfo
{
	public:
	std::string name;
	std::string msg;
};


class RsConfig
{
	public:
	std::list<std::string>   sharedDirList;
	std::string		 incomingDir;

	std::string		localAddr;
	int			localPort;
	std::string		extAddr;
	int			extPort;
	std::string		extName;

	bool			firewalled;
	bool			forwardPort;

	int			maxDataRate;     /* kb */
	int			maxIndivDataRate; /* kb */

	int			promptAtBoot; /* popup the password prompt */
};

/********************** For Search Interface *****************/

/* This is still rough, implement later! */

	/* text based ones */
const std::string TypeExt  = "ext";
const std::string TypeName = "name";
const std::string TypeHash = "hash";
const std::string TypeSize = "size";

const int OpContains    = 0x001;
const int OpExactMatch  = 0x002;
const int OpLessThan    = 0x003;
const int OpGreaterThan = 0x004;

class Condition
{
	public:

	std::string type;
	int op;
	double value;
	std::string name;
};

class SearchRequest
{
	public:
	int searchId;	
	RsCertId toId;  /* all zeros for everyone! */
	std::list<Condition> tests;
};


std::ostream &operator<<(std::ostream &out, const NeighbourInfo &info);
std::ostream &operator<<(std::ostream &out, const MessageInfo &info);
std::ostream &operator<<(std::ostream &out, const ChannelInfo &info);
std::ostream &operator<<(std::ostream &out, const ChatInfo &info);
std::ostream &operator<<(std::ostream &out, const PersonInfo &info);
std::ostream &print(std::ostream &out, const DirInfo &info, int indentLvl);


#endif


