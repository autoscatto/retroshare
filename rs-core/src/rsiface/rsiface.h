#ifndef RETROSHARE_GUI_INTERFACE_H
#define RETROSHARE_GUI_INTERFACE_H

/*
 * "$Id: rsiface.h,v 1.9 2007-04-21 19:08:51 rmf24 Exp $"
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


#include "rstypes.h"

#include <map>

class NotifyBase;
class RsIface;
class RsControl;
//class RsServer;
class RsInit;

/* declare single RsIface for everyone to use! */

extern RsIface   *rsiface;
extern RsControl *rsicontrol;

/* RsInit -> Configuration Parameters for RetroShare Startup
 */

RsInit *InitRsConfig();
const char   *RsConfigDirectory(RsInit *config);
void    CleanupRsConfig(RsInit *);


// Called First... (handles comandline options) 
int InitRetroShare(int argc, char **argv, RsInit *config);

// This Functions are used for Login.
bool ValidateCertificate(RsInit *config, std::string &userName);
bool ValidateTrustedUser(RsInit *config, std::string fname, std::string &userName);
bool LoadPassword(RsInit *config, std::string passwd);
bool RsGenerateCertificate(RsInit *config, std::string name, std::string org,
                        std::string loc, std::string country, std::string passwd, std::string &errString);

/* Auto Login Fns */
bool  RsTryAutoLogin(RsInit *config);
bool  RsStoreAutoLogin(RsInit *config);
bool  RsClearAutoLogin(std::string basedir);

// Handle actual Login.
int LoadCertificates(RsInit *config, bool autoLoginNT);

RsIface   *createRsIface  (NotifyBase &notify);
RsControl *createRsControl(RsIface &iface, NotifyBase &notify);


class RsIface /* The Main Interface Class - create a single one! */
{
public:
	RsIface(NotifyBase &callback)
	:cb(callback) { return; }
	virtual ~RsIface() { return; }

/****************************************/

	/* Stubs for Very Important Fns -> Locking Functions */
virtual	void lockData() = 0;
virtual	void unlockData() = 0;

	const std::map<RsCertId,NeighbourInfo> &getNeighbourMap() 
		{ return mNeighbourMap; }

	const std::map<RsCertId,NeighbourInfo> &getFriendMap()
		{ return mFriendMap; }

	const NeighbourInfo * getFriend(std::string id);

	const std::list<FileTransferInfo> &getTransferList()
		{ return mTransferList; }

	const std::list<PersonInfo> &getRemoteDirectoryList()
		{ return mRemoteDirList; }

	const std::list<PersonInfo> &getLocalDirectoryList()
		{ return mLocalDirList; }

	const PersonInfo *getPerson(std::string id);
	const DirInfo *getDirectory(std::string id, std::string path);

	const std::list<MessageInfo> &getMessages()
		{ return mMessageList; }

	const std::map<RsChanId, ChannelInfo> &getChannels()
		{ return mChannelMap; }

	const std::map<RsChanId, ChannelInfo> &getOurChannels()
		{ return mChannelOwnMap; }

	const MessageInfo *getMessage(std::string cId, std::string mId);
	const MessageInfo *getChannelMsg(std::string chId, std::string mId);

	std::list<ChatInfo> getChatNew()
		{ 
			std::list<ChatInfo> newList = mChatList; 
			mChatList.clear(); 
			return newList;
		}

	const std::list<FileInfo> &getRecommendList()
		{ return mRecommendList; }

	const RsConfig &getConfig()
		{ return mConfig; }
/****************************************/


	/* Flags to indicate used or not */
	enum DataFlags
	{
		Neighbour = 0,
		Friend = 1,
		DirLocal = 2,  /* Not Used - QModel instead */
		DirRemote = 3, /* Not Used - QModel instead */
		Transfer = 4,
		Message = 5,
		Channel = 6,
		Chat = 7,
		Recommend = 8,
		Config = 9,
		NumOfFlags = 10
	};


	/* 
	 * Operations for flags
	 */

bool	setChanged(DataFlags set); /* set to true */
bool	getChanged(DataFlags set); /* leaves it */
bool	hasChanged(DataFlags set); /* resets it */

	private:

	/* Internal Fn for getting the Directory Entry */
	PersonInfo *getPersonMod(std::string id);
	DirInfo *getDirectoryMod(std::string id, std::string path);

void	fillLists(); /* create some dummy data to display */

		/* Internals */
	std::map<RsCertId,NeighbourInfo> mNeighbourMap;
	std::map<RsCertId,NeighbourInfo> mFriendMap;
	std::list<PersonInfo>    mRemoteDirList;
	std::list<PersonInfo>    mLocalDirList;
	std::list<FileTransferInfo> mTransferList;
	std::list<MessageInfo>   mMessageList;
	std::map<RsChanId, ChannelInfo> mChannelMap;
	std::map<RsChanId, ChannelInfo> mChannelOwnMap;
	std::list<ChatInfo>      mChatList;
	std::list<FileInfo>      mRecommendList;

	bool mChanged[NumOfFlags];

	RsConfig mConfig;

	NotifyBase &cb;

	/* Classes which can update the Lists! */
	friend class RsControl;
	friend class RsServer; 
};


class RsControl /* The Main Interface Class - for controlling the server */
{
	public:

	RsControl(RsIface &i, NotifyBase &callback)
	:cb(callback), rsIface(i) { return; }

virtual ~RsControl() { return; }

	/* Real Startup Fn */
virtual int StartupRetroShare(RsInit *config) = 0;

/****************************************/
	/* Neighbour Operations */
virtual	std::string NeighGetInvite() = 0;
virtual	int NeighLoadPEMString(std::string pem, std::string &id)  = 0;
virtual	int NeighLoadCertificate(std::string fname, std::string &id)  = 0;
virtual	int NeighAuthFriend(std::string id, RsAuthId code)   = 0;
virtual	int NeighAddFriend(std::string id)                   = 0;
virtual int NeighGetSigners(std::string uid, char *out, int len) = 0;

/****************************************/
	/* Friend Operations */
virtual	int FriendStatus(std::string id, bool accept)        = 0;
virtual	int FriendRemove(std::string id)                     = 0;
virtual	int FriendConnectAttempt(std::string id)             = 0;

virtual	int FriendSignCert(std::string id)                   = 0;
virtual	int FriendTrustSignature(std::string id, bool trust) = 0;

virtual	int FriendSetLocalAddress(std::string id, std::string addr,
		unsigned short port)                      = 0;
virtual	int FriendSetExtAddress(std::string id, std::string addr,
		unsigned short port)                      = 0;
virtual	int FriendSetDNSAddress(std::string id, std::string addr) = 0;
virtual	int FriendSetFirewall(std::string id, bool firewalled, bool forwarded) = 0;

virtual	int FriendSaveCertificate(std::string id, std::string fname) = 0;

/*
virtual	int FriendSetConnectMode(std::string id, int mode)   = 0;
*/
virtual	int FriendSetBandwidth(std::string id, float outkB, float inkB) = 0;

/****************************************/
	/* Directory Actions */
virtual int RequestDirectories(std::string uid, std::string path, int depth) = 0;
// Not sure this should be in the interface.
//virtual	int DirectoryRequestUpdate(std::string id, std::string dir) = 0;

/****************************************/
	/* Actions For Upload/Download */

virtual	int FileRequest(std::string uId, std::string src, std::string dest, int size) = 0;
virtual	int FileRecommend(std::string uId, std::string fname, int size) = 0;

// Transfer control.
virtual	int FileCancel(std::string uId, std::string fname)    = 0;
virtual int FileClearCompleted()                              = 0;
virtual	int FileSetBandwidthTotals(float outkB, float inkB)   = 0;

// removing local files.
virtual	int FileDelete(std::string uId, std::string fname)    = 0;


//virtual int FileMove(std::string uId, std::string src, std::string dest) = 0;

/****************************************/
	/* Message Items */
virtual	int MessageSend(MessageInfo &info)                 = 0;
virtual int MessageDelete(std::string id)                  = 0;
virtual int MessageRead(std::string id)                    = 0;

	/* Channel Items */
virtual	int ChannelCreateNew(ChannelInfo &info)            = 0;
virtual	int ChannelSendMsg(ChannelInfo &info)              = 0;

/****************************************/
	/* Chat */
virtual	int 	ChatSend(ChatInfo &ci)                     = 0;

/****************************************/

	/* Flagging Persons / Channels / Files in or out of a set (CheckLists) */
virtual int 	SetInChat(std::string id, bool in) = 0;		/* friend : chat msgs */
virtual int 	SetInMsg(std::string id, bool in)  = 0;		/* friend : msg receipients */
virtual int 	SetInBroadcast(std::string id, bool in) = 0;	/* channel : channel broadcast */
virtual int 	SetInSubscribe(std::string id, bool in) = 0;	/* channel : subscribed channels */
virtual int 	SetInRecommend(std::string id, bool in) = 0;	/* file : recommended file */

/****************************************/
        /* RsIface Networking */
virtual int     NetworkDHTActive(bool active)  = 0;
virtual int     NetworkUPnPActive(bool active) = 0;

/****************************************/
	/* Config */
virtual	int 	ConfigAddSharedDir( std::string dir )      = 0;
virtual	int 	ConfigRemoveSharedDir( std::string dir )   = 0;
virtual	int 	ConfigSetIncomingDir( std::string dir )    = 0;

virtual	int 	ConfigSetLocalAddr( std::string ipAddr, int port ) = 0;
virtual	int 	ConfigSetExtAddr( std::string ipAddr, int port ) = 0;
virtual	int 	ConfigSetExtName( std::string addr ) = 0;
virtual int     ConfigSetLanConfig( bool fire, bool forw ) = 0;

virtual int     ConfigSetDataRates( int total, int indiv ) = 0;
virtual	int 	ConfigSetBootPrompt( bool on ) = 0;
virtual	int 	ConfigSave( ) = 0;

/****************************************/

	NotifyBase &getNotify() { return cb; }
	RsIface    &getIface()  { return rsIface; }

	private:
	NotifyBase &cb;
	RsIface    &rsIface;
};


/********************** Overload this Class for callback *****************/


class NotifyBase
{
	public:
	NotifyBase() { return; }
	virtual ~NotifyBase() { return; }
	virtual void notifyListPreChange(int list, int type) { (void) list; (void) type; return; }
	virtual void notifyListChange(int list, int type) { (void) list; (void) type; return; }
	virtual void notifyErrorMsg(int list, int sev, std::string msg) { (void) list; (void) sev; (void) msg; return; }
	virtual void notifyChat() { return; }
};

const int NOTIFY_LIST_NEIGHBOURS = 1;
const int NOTIFY_LIST_FRIENDS = 2;
const int NOTIFY_LIST_DIRLIST = 3;
const int NOTIFY_LIST_SEARCHLIST = 4;
const int NOTIFY_LIST_MESSAGELIST = 5;
const int NOTIFY_LIST_CHANNELLIST = 6;
const int NOTIFY_LIST_TRANSFERLIST = 7;

const int NOTIFY_TYPE_SAME   = 0x01;
const int NOTIFY_TYPE_MOD    = 0x02; /* general purpose, check all */
const int NOTIFY_TYPE_ADD    = 0x04; /* flagged additions */
const int NOTIFY_TYPE_DEL    = 0x08; /* flagged deletions */



#endif
