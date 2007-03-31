#ifndef RETROSHARE_GUI_INTERFACE_H
#define RETROSHARE_GUI_INTERFACE_H

#include "rstypes.h"

class NotifyBase;
class RsIface;

/* declare single RsIface for everyone to use! */

extern RsIface *rsiface;


class RsIface /* The Main Interface Class - create a single one! */
{
public:
	RsIface(NotifyBase &callback);
	virtual ~RsIface();

/****************************************/
	/* Stubs for Very Important Fns -> Locking Functions */
	void lockData() { return; }
	void unlockData() { return; }

/****************************************/
	/* Neighbour Panel */
	const std::list<NeighbourInfo> &getNeighbourList();

	/* Neighbour Operations */
	int  NeighLoadCertficate(std::string fname);
	int  NeighAuthRequest(CertId id, AuthId code);


/****************************************/
	/* Friend Panel */

	const std::list<NeighbourInfo> &getFriendList();

	/* Friend Operations */
	int FriendRemove(CertId id);
	int FriendConnectAttempt(CertId id);
	int FriendTrustSignature(CertId id);

	int FriendSetAddress(CertId, std::string &addr);
	int FriendSaveCertificate(CertId id, std::string fname);

	int FriendSetConnectMode(CertId id, int mode);
	int FriendSetBandwidth(CertId id, float outkB, float inkB);

/****************************************/
	/* Download/Upload Files */

	const std::list<FileTransferInfo> &getTransferList();

	/* Actions For Upload/Download */
	int FileRequest(CertId uId, std::string fname);
	int FileCancel(CertId uId, std::string fname);

	int FileSetBandwidthTotals(float outkB, float inkB);

/****************************************/
	/* Directory Items */
	const std::list<PersonInfo> &getRemoteDirectoryList();
	const std::list<PersonInfo> &getLocalDirectoryList();


	/* go directly for a directory
	 * eg. if we expect it to change.
	 *
	 * TODO.
	 */
	const DirInfo &getDirectory(CertId, std::string path);

	/* Directory Actions */
	int DirectoryRequestUpdate(CertId uId, std::string dir);

/****************************************/
	/* Message Items */
	const std::list<MessageInfo> &getMessages();

	int MessageSend(MessageInfo &info);
	int MessageDelete(MessageInfo &info);

	/* Channel Items */
	const std::list<ChannelInfo> &getChannels();
	const std::list<ChannelInfo> &getOurChannels();

	int ChannelCreateNew(ChannelInfo &info);
	int ChannelSendMsg(ChannelInfo &info);

/****************************************/
	/* Chat */
	std::list<ChatInfo> getChatNew();
	int 	ChatSend(ChatInfo &ci);

/****************************************/

	private:

void	fillLists(); /* create some dummy data to display */

		/* Internals */
	std::list<NeighbourInfo> mNeighbourList;
	std::list<NeighbourInfo> mFriendList;
	std::list<PersonInfo>    mRemoteDirList;
	std::list<PersonInfo>    mLocalDirList;
	std::list<FileTransferInfo> mTransferList;
	std::list<MessageInfo>   mMessageList;
	std::list<ChannelInfo>   mChannelList;
	std::list<ChannelInfo>   mChannelOwnList;
	std::list<ChatInfo>      mChatList;

	NotifyBase &cb;
};


/********************** Overload this Class for callback *****************/


class NotifyBase
{
	public:
	NotifyBase() { return; }
	virtual ~NotifyBase() { return; }
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

const int NOTIFY_TYPE_SAME   = 0x01;
const int NOTIFY_TYPE_MOD    = 0x02; /* general purpose, check all */
const int NOTIFY_TYPE_ADD    = 0x04; /* flagged additions */
const int NOTIFY_TYPE_DEL    = 0x08; /* flagged deletions */



#endif
