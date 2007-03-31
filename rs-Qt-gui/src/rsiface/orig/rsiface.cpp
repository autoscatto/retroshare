
/* Insides of RetroShare interface.
 * only prints stuff out at the moment
 */

#include "rsiface.h"
#include <iostream>
#include <sstream>

RsIface::	RsIface(NotifyBase &callback)
	:cb(callback)
	{
		fillLists();
	}

RsIface::~RsIface()
	{
		return;
	}
/****************************************/
	/* Neighbour Panel */
const std::list<NeighbourInfo> &RsIface::getNeighbourList()
{
	std::cerr << "getNeighbourList() request";
	std::cerr << std::endl;

	return mNeighbourList;
}

	/* Neighbour Operations */
int  RsIface::NeighLoadCertficate(std::string fname)
{
	std::cerr << "NeighLoadCertificate():";
	std::cerr << std::endl;
	std::cerr << " FileName: " << fname;
	std::cerr << std::endl;

	return 1;
}


int  RsIface::NeighAuthRequest(CertId id, AuthId code)
{
	std::cerr << "FriendSaveCertificate():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << " AuthId: " << code;
	std::cerr << std::endl;

	return 1;
}



/****************************************/
	/* Friend Panel */

const std::list<NeighbourInfo> &RsIface::getFriendList()
{
	std::cerr << "getFriendList() request";
	std::cerr << std::endl;

	return mFriendList;
}


	/* Friend Operations */
int RsIface::FriendRemove(CertId id)
{
	std::cerr << "FriendRemove():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << std::endl;

	return 1;
}

int RsIface::FriendConnectAttempt(CertId id)
{
	std::cerr << "FriendConnectAttempt():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << std::endl;

	return 1;
}

int RsIface::FriendTrustSignature(CertId id)
{
	std::cerr << "FriendTrustSignature():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << std::endl;

	return 1;
}


int RsIface::FriendSetAddress(CertId id, std::string &addr)
{
	std::cerr << "FriendSetAddress():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << std::endl;
	std::cerr << "Address: " << addr;
	std::cerr << std::endl;

	return 1;
}


int RsIface::FriendSaveCertificate(CertId id, std::string fname)
{
	std::cerr << "FriendSaveCertificate():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << " FileName: " << fname;
	std::cerr << std::endl;

	return 1;
}

int RsIface::FriendSetConnectMode(CertId id, int mode)
{
	std::cerr << "FriendSetConnectMode():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << " Mode: " << mode;
	std::cerr << std::endl;

	return 1;
}


int RsIface::FriendSetBandwidth(CertId id, float outkB, float inkB)
{
	std::cerr << "FriendSetBandwidth():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << id;
	std::cerr << " Out: " << outkB;
	std::cerr << " In: " << inkB;
	std::cerr << std::endl;

	return 1;
}


/****************************************/
	/* Download/Upload Files */

const std::list<FileTransferInfo> &RsIface::getTransferList()
{
	std::cerr << "getTransferList() request";
	std::cerr << std::endl;

	return mTransferList;
}


	/* Actions For Upload/Download */
int RsIface::FileRequest(CertId uId, std::string fname)
{
	std::cerr << "FileRequest():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << uId;
	std::cerr << " FileName: " << fname;
	std::cerr << std::endl;

	return 1;
}


int RsIface::FileCancel(CertId uId, std::string fname)
{
	std::cerr << "FileCancel():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << uId;
	std::cerr << " FileName: " << fname;
	std::cerr << std::endl;

	return 1;
}


int RsIface::FileSetBandwidthTotals(float outkB, float inkB)
{
	std::cerr << "FileSetBandwidthTotals():";
	std::cerr << std::endl;
	std::cerr << "Out: " << outkB;
	std::cerr << " In: " << inkB;
	std::cerr << std::endl;

	return 1;
}


/****************************************/
	/* Directory Items */



const std::list<PersonInfo> &RsIface::getRemoteDirectoryList()
{
	std::cerr << "getRemoteDirList() request";
	std::cerr << std::endl;
	return mRemoteDirList;
}


const std::list<PersonInfo> &RsIface::getLocalDirectoryList()
{
	std::cerr << "getLocalDirList() request";
	std::cerr << std::endl;
	return mLocalDirList;
}


	/* Directory Actions */
const DirInfo &getDirectory(CertId, std::string path)
{
	/* very bad, but will compile? */
	DirInfo tmp;
	return tmp;
}


int RsIface::DirectoryRequestUpdate(CertId uId, std::string dir)
{
	std::cerr << "DirectoryRequestUpdate():";
	std::cerr << std::endl;
	std::cerr << "CertId: " << uId;
	std::cerr << " Dir: " << dir;
	std::cerr << std::endl;

	return 1;
}



/****************************************/
	/* Message Items */
const std::list<MessageInfo> &RsIface::getMessages()
{
	std::cerr << "getMessages() request";
	std::cerr << std::endl;
	return mMessageList;
}


int RsIface::MessageSend(MessageInfo &info)
{
	std::cerr << "MessageSend():";
	std::cerr << std::endl;
	std::cerr << info;
	std::cerr << std::endl;

	return 1;
}

int RsIface::MessageDelete(MessageInfo &info)
{
	std::cerr << "MessageDelete():";
	std::cerr << std::endl;
	std::cerr << info;
	std::cerr << std::endl;

	return 1;
}

	/* Channel Items */
const std::list<ChannelInfo> &RsIface::getChannels()
{
	std::cerr << "getChannels() request";
	std::cerr << std::endl;
	return mChannelList;
}

const std::list<ChannelInfo> &RsIface::getOurChannels()
{
	std::cerr << "getOurChannels()";
	std::cerr << std::endl;
	return mChannelOwnList;
}


int RsIface::ChannelCreateNew(ChannelInfo &info)
{
	std::cerr << "ChannelSendMsg():";
	std::cerr << std::endl;
	std::cerr << info;
	std::cerr << std::endl;

	return 1;
}

int RsIface::ChannelSendMsg(ChannelInfo &info)
{
	std::cerr << "ChannelSendMsg():";
	std::cerr << std::endl;
	std::cerr << info;
	std::cerr << std::endl;

	return 1;
}


/****************************************/
	/* Chat */
std::list<ChatInfo> RsIface::getChatNew()
{
	std::cerr << "getChatNew()";
	std::cerr << std::endl;
	return mChatList;
}

int 	RsIface::ChatSend(ChatInfo &info)
{
	std::cerr << "ChatSend():";
	std::cerr << std::endl;
	std::cerr << info;
	std::cerr << std::endl;

	return 1;
}

/****************************************/
	/* Print Functions for Info Classes */
std::ostream &operator<<(std::ostream &out, const NeighbourInfo &info)
{
	out << "Neighbour Name: " << info.name;
	out << std::endl;
	out << "TrustLvl:  " << info.trustLvl;
	out << std::endl;
	out << "Status:    " << info.status;
	out << std::endl;
	out << "Auth Code: " << info.authCode;
	out << std::endl;
	return out;
}

std::ostream &operator<<(std::ostream &out, const PersonInfo &info)
{
	out << "Directory Listing for: " << info.name;
	out << std::endl;
	print(out, info.rootdir, 0);
	return out;
}

std::ostream &print(std::ostream &out, const DirInfo &info, int indentLvl)
{
	int i;
	std::ostringstream indents;
	for(i = 0; i < indentLvl; i++)
	{
		indents << "  ";
	}
	out << indents.str() << "Dir: " << info.dirname << std::endl;
	if (info.subdirs.size() > 0)
	{
	  out << indents.str() << "subdirs:" << std::endl;
	  std::list<DirInfo>::const_iterator it;
	  for(it = info.subdirs.begin(); it != info.subdirs.end(); it++)
	  {
	  	print(out, *it, indentLvl + 1);
	  }
	}
	if (info.files.size() > 0)
	{
	  out << indents.str() << "files:" << std::endl;
	  std::list<FileInfo>::const_iterator it2;
	  for(it2 = info.files.begin(); it2 != info.files.end(); it2++)
	  {
	        out << indents.str() << "  " << it2->fname;
		out << " " << it2->size << std::endl;
	  }
	}
	return out;
}


std::ostream &operator<<(std::ostream &out, const MessageInfo &info)
{
	out << "MessageInfo(TODO)";
	out << std::endl;
	return out;
}

std::ostream &operator<<(std::ostream &out, const ChannelInfo &info)
{
	out << "ChannelInfo(TODO)";
	out << std::endl;
	return out;
}

std::ostream &operator<<(std::ostream &out, const ChatInfo &info)
{
	out << "ChatInfo(TODO)";
	out << std::endl;
	return out;
}


/****************************************/

void 	RsIface::fillLists()
{
	/* create some dummy Info to print! */
	NeighbourInfo n1, n2, n3;
	NeighbourInfo f1, f2, f3;

	n1.name = "Joe Doe";
	n1.trustLvl = "Trusted";
	n1.status = "OffLine";

	n2.name = "Sammy Small";
	n2.trustLvl = "Unknown";
	n2.status = "Online";

	n3.name = "Jacki Chan";
	n3.trustLvl = "Authenticated";
	n3.status = "OffLine";


	f1.name = "Best Friend 1";
	f1.trustLvl = "Trusted";
	f1.status = "Online";

	f2.name = "Jane Doe";
	f2.trustLvl = "Authenticated";
	f2.status = "Online";

	f3.name = "Paul Smith";
	f3.trustLvl = "Authenticated";
	f3.status = "OffLine";

	mNeighbourList.push_back(n1);
	mNeighbourList.push_back(n2);
	mNeighbourList.push_back(n3);

	mFriendList.push_back(f1);
	mFriendList.push_back(f2);
	mFriendList.push_back(f3);

	/* now make a example file listings */

	PersonInfo p1, p2;
	DirInfo p1d1, p1d2, p1d3, p1d4;
	DirInfo p2d1, p2d2, p2d3, p2d4;
	FileInfo fl1, fl2, fl3, fl4, fl5;

	fl1.fname = "File1";
	fl1.size  = 1024000;
	fl2.fname = "File2";
	fl2.size  = 7890;
	fl3.fname = "File3";
	fl3.size  = 12345;
	fl4.fname = "File4";
	fl4.size  = 256;
	fl5.fname = "File5";
	fl5.size  = 102400;

	p1d1.dirname = "Dir1";
	p1d2.dirname = "Dir2";
	p1d3.dirname = "Dir3";
	p1d4.dirname = "Dir4";


	p1d4.files.push_back(fl1);

	p1d3.subdirs.push_back(p1d4);
	p1d3.files.push_back(fl2);

	p1d2.files.push_back(fl3);
	p1d2.files.push_back(fl4);

	p1d1.subdirs.push_back(p1d2);
	p1d1.subdirs.push_back(p1d3);
	p1d1.files.push_back(fl5);

	p1.name = "Own Files";
	p1.rootdir = p1d1;


	p2d1.dirname = "DiffDir1";
	p2d2.dirname = "DiffDir2";
	p2d3.dirname = "DiffDir3";
	p2d4.dirname = "DiffDir4";


	p2d4.files.push_back(fl1);
	p2d4.files.push_back(fl2);
	p2d4.files.push_back(fl3);
	p2d4.files.push_back(fl4);
	p2d4.files.push_back(fl5);

	p2d3.subdirs.push_back(p2d4);
	p2d2.subdirs.push_back(p2d3);
	p2d1.subdirs.push_back(p2d2);

	p2.name = "Jane Doe";
	p2.rootdir = p2d1;

	mRemoteDirList.push_back(p2);

	// duplicate to Local.
	mLocalDirList.push_back(p1);

}



