#ifndef RETROSHARE_GUI_INTERFACE_H
#define RETROSHARE_GUI_INTERFACE_H

/*
 * "$Id$"
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
void    CleanupRsConfig(RsInit *);

int InitRetroShare(int argc, char **argv, RsInit *config);
int LoadCertificates(RsInit *config);

RsControl *createRsControl(RsIface &iface, NotifyBase &notify);


class RsIface /* The Main Interface Class - create a single one! */
{
public:
	RsIface(NotifyBase &callback)
	:cb(callback) { return; }
	virtual ~RsIface() { return; }

/****************************************/

	/* Stubs for Very Important Fns -> Locking Functions */
	void lockData() { return; }
	void unlockData() { return; }

	const std::map<RsCertId,NeighbourInfo> &getNeighbourMap() 
		{ return mNeighbourMap; }

	const std::map<RsCertId,NeighbourInfo> &getFriendMap()
		{ return mFriendMap; }

	const std::list<FileTransferInfo> &getTransferList()
		{ return mTransferList; }

	const std::list<PersonInfo> &getRemoteDirectoryList()
		{ return mRemoteDirList; }

	const std::list<PersonInfo> &getLocalDirectoryList()
		{ return mLocalDirList; }

	const DirInfo &getDirectory(RsCertId, std::string path);

	const std::list<MessageInfo> &getMessages()
		{ return mMessageList; }

	const std::map<RsChanId, ChannelInfo> &getChannels()
		{ return mChannelMap; }

	const std::map<RsChanId, ChannelInfo> &getOurChannels()
		{ return mChannelOwnMap; }

	std::list<ChatInfo> getChatNew()
		{ return mChatList; }

/****************************************/

	private:

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
virtual	int NeighLoadCertficate(std::string fname)        = 0;
virtual	int NeighAuthFriend(RsCertId id, RsAuthId code)   = 0;
virtual	int NeighAddFriend(RsCertId id)                   = 0;

/****************************************/
	/* Friend Operations */
virtual	int FriendStatus(RsCertId id, bool accept)        = 0;
virtual	int FriendRemove(RsCertId id)                     = 0;
virtual	int FriendConnectAttempt(RsCertId id)             = 0;

virtual	int FriendSignCert(RsCertId id)                   = 0;
virtual	int FriendTrustSignature(RsCertId id, bool trust) = 0;

virtual	int FriendSetAddress(RsCertId, std::string &addr,
		unsigned short port)                      = 0;
virtual	int FriendSaveCertificate(RsCertId id, std::string fname) = 0;

/*
virtual	int FriendSetConnectMode(RsCertId id, int mode)   = 0;
*/
virtual	int FriendSetBandwidth(RsCertId id, float outkB, float inkB) = 0;

/****************************************/
	/* Directory Actions */
virtual	int DirectoryRequestUpdate(std::string id, std::string dir) = 0;

/****************************************/
	/* Actions For Upload/Download */
virtual	int FileRequest(RsCertId uId, std::string src, std::string dest) = 0;
virtual	int FileCancel(RsCertId uId, std::string fname)    = 0;
virtual	int FileDelete(RsCertId uId, std::string fname)    = 0;

virtual	int FileSetBandwidthTotals(float outkB, float inkB) = 0;

//virtual int FileMove(RsCertId uId, std::string src, std::string dest) = 0;

/****************************************/
	/* Message Items */
virtual	int MessageSend(MessageInfo &info)                 = 0;
virtual	int MessageDelete(MessageInfo &info)               = 0;

	/* Channel Items */
virtual	int ChannelCreateNew(ChannelInfo &info)            = 0;
virtual	int ChannelSendMsg(ChannelInfo &info)              = 0;

/****************************************/
	/* Chat */
virtual	int 	ChatSend(ChatInfo &ci)                     = 0;

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
	virtual void notifyListPreChange(int list, int type) { return; }
	virtual void notifyListChange(int list, int type) { return; }
	virtual void notifyErrorMsg(int list, int sev, std::string msg) { return; }
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
