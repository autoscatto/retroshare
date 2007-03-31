#ifndef MRK_P3RS_INTERFACE_H
#define MRK_P3RS_INTERFACE_H

/*
 * "$Id: p3face.h,v 1.6 2007-03-21 18:45:41 rmf24 Exp $"
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

#include "server/filedexserver.h"
#include "pqi/pqipersongrp.h"
#include "pqi/pqissl.h"

#include "pqi/p3disc.h"

#include "rsiface/rsiface.h"
#include "rsiface/rstypes.h"
#include "util/rsthreads.h"

/* The Main Interface Class - for controlling the server */

/* The init functions are actually Defined in p3face-startup.cc
 */
RsInit *InitRsConfig();
void    CleanupRsConfig(RsInit *);
int InitRetroShare(int argc, char **argv, RsInit *config);
int LoadCertificates(RsInit *config);

RsControl *createRsControl(RsIface &iface, NotifyBase &notify);


class PendingDirectory
{
        public:
        PendingDirectory(RsCertId in_id, const DirInfo *req_dir, int depth);
void	addEntry(PQFileItem *item);	

        RsCertId id;
        int reqDepth;
        int reqTime;
        DirInfo data;
};


class RsServer: public RsControl, public RsThread
{
	public:
/****************************************/
	/* p3face-startup.cc: init... */
virtual	int StartupRetroShare(RsInit *config);

	public:
/****************************************/
	/* p3face.cc: main loop / util fns / locking. */

	RsServer(RsIface &i, NotifyBase &callback);
	virtual ~RsServer();

        /* Thread Fn: Run the Core */
virtual void run();

	private:
	/* locking stuff */
void    lockRsCore() { coreMutex.lock(); }
void    unlockRsCore() { coreMutex.unlock(); }

	/* mutex */
	RsMutex coreMutex;

	/* General Internal Helper Functions
	   (Must be Locked)
         */

cert   *intFindCert(RsCertId &id);
RsChanId intGetCertId(cert *c);

/****************************************/
	/* p3face-people Operations */

	public:

	// All Public Fns, must use td::string instead of RsCertId.
	// cos of windows interfacing errors...

virtual int NeighLoadCertificate(std::string fname);
virtual int NeighAuthFriend(std::string id, RsAuthId code);
virtual int NeighAddFriend(std::string id); 

	/* Friend Operations */
virtual int FriendStatus(std::string id, bool accept);

virtual int FriendRemove(std::string id);
virtual int FriendConnectAttempt(std::string id);

virtual int FriendSignCert(std::string id);
virtual int FriendTrustSignature(std::string id, bool trust);

//virtual int FriendSetAddress(std::string, std::string &addr, unsigned short port);
virtual int FriendSaveCertificate(std::string id, std::string fname);
virtual int FriendSetBandwidth(std::string id, float outkB, float inkB);

virtual int FriendSetLocalAddress(std::string id, std::string addr, unsigned short port);
virtual int FriendSetExtAddress(std::string id, std::string addr, unsigned short port);
virtual int FriendSetDNSAddress(std::string id, std::string addr);
virtual int FriendSetFirewall(std::string id, bool firewalled, bool forwarded);

	private:

void    intNotifyCertError(RsCertId &id, std::string errstr);
void    intNotifyChangeCert(RsCertId &id);

	/* Internal Update Iface Fns */
int 	UpdateAllCerts();
int 	UpdateAllNetwork();

void    initRsNI(cert *c, NeighbourInfo &ni); /* translate to Ext */

int 	ensureExtension(std::string &name, std::string def_ext);

	/* for DHT stuff */
int 	InitDHT(std::string);
int 	CheckDHT();

/****************************************/
/****************************************/
	/* p3face-file Operations */

	public:

	/* Directory Actions */
virtual int RequestDirectories(std::string uid, std::string path, int depth);

	/* Actions For Upload/Download */
virtual int FileRecommend(std::string uId, std::string src, int size);
virtual int FileBroadcast(std::string uId, std::string src, int size);

virtual int FileRequest(std::string uId, std::string src, std::string dest, int size);
virtual int FileDelete(std::string, std::string);


virtual int FileCancel(std::string uId, std::string fname);
virtual int FileClearCompleted();


virtual int FileSetBandwidthTotals(float outkB, float inkB);

	private:

	/* this next is lower now (should be made private? */
virtual int DirectoryRequestUpdate(RsCertId id, std::string dir);

int 	UpdateAllTransfers();

	/* send requests to people */
int     UpdateRemotePeople();

#if (0)
	/****************** OLD DIR UPDATE SYSTEM *********/
	/* Internal Update Iface Fns */
int 	UpdateAllFiles();

	/* Lock Fns */
int     UpdateSubdir(DirNode * dir, DirInfo &di);

int     initRsPi(DirBase *dir, PersonInfo &pi);
int     initRsDi(DirNode *dir, DirInfo &di);
int     initRsFi(PQFileItem *file, FileInfo &fi);
	/****************** OLD DIR UPDATE SYSTEM *********/
#endif


	/* New Update Dir Systems */

	/* Fn's that handle locking ..... */
int 	UpdateDirectories(); /* Periodically Called. */



	/* Either... Locked/Unlocked - dependent on the Data ***/
bool 	DirectoryUpToDate(const DirInfo *dir);

	/* Internal Data for updates ****/

unsigned int	nextFileMId();
unsigned int	lastFileMId; /* storage */

	std::list<PendingDirectory> pendingDirs;


/****************************************/
/****************************************/
	/* p3face-msg Operations */

	public:
	/* Message Items */
virtual int MessageSend(MessageInfo &info);
virtual int MessageDelete(MessageInfo &info);

	/* Channel Items */
virtual int ChannelCreateNew(ChannelInfo &info);
virtual int ChannelSendMsg(ChannelInfo &info);

	/* Chat */
virtual int 	ChatSend(ChatInfo &ci);

/* Flagging Persons / Channels / Files in or out of a set (CheckLists) */
virtual int     SetInChat(std::string id, bool in);         /* friend : chat msgs */
virtual int     SetInMsg(std::string id, bool in);          /* friend : msg receipients */ 
virtual int     SetInBroadcast(std::string id, bool in);    /* channel : channel broadcast */
virtual int     SetInSubscribe(std::string id, bool in);    /* channel : subscribed channels */
virtual int     SetInRecommend(std::string id, bool in);    /* file : recommended file */


	private:

	/* Internal Update Iface Fns */
int 	UpdateAllChat();
int 	UpdateAllMsgs();
int 	UpdateAllChannels();

	/* Internal Helper Fns */
RsChanId signToChanId(const channelSign &cs) const;
void initRsChatInfo(ChatItem *c, ChatInfo &i);

int intAddChannel(ChannelInfo &info);
int intAddChannelMsg(RsChanId id, MessageInfo &msg);

void initRsCI(pqichannel *in, ChannelInfo &out);
void initRsCMI(pqichannel *chan, channelMsg *cm, MessageInfo &msg);
void initRsCMFI(pqichannel *chan, chanMsgSummary *msg,
      const PQChanItem::FileItem *cfdi, FileInfo &file);

void intCheckFileStatus(FileInfo &file);

void initRsMI(MsgItem *msg, MessageInfo &mi);

/****************************************/
/****************************************/


	public:
/****************************************/
	/* RsIface Config */
        /* Config */
virtual int     ConfigAddSharedDir( std::string dir );
virtual int     ConfigRemoveSharedDir( std::string dir );
virtual int     ConfigSetIncomingDir( std::string dir );

virtual int     ConfigSetLocalAddr( std::string ipAddr, int port );
virtual int     ConfigSetExtAddr( std::string ipAddr, int port );
virtual int     ConfigSetExtName( std::string addr );
virtual int     ConfigSetLanConfig( bool fire, bool forw );

virtual int     ConfigSetDataRates( int total, int indiv );
virtual int     ConfigSetBootPrompt( bool on );
virtual int     ConfigSave( );

	private:
int UpdateAllConfig();

/****************************************/


	private: 

	// The real Server Parts.

	filedexserver *server;
	pqipersongrp *pqih;
	sslroot *sslr;
	p3disc *ad;

	// Worker Data.....

};

/* Helper function to convert windows paths
 * into unix (ie switch \ to /) for FLTK's file chooser
 */

std::string make_path_unix(std::string winpath);



/* Initialisation Class (not publicly disclosed to RsIFace) */

class RsInit
{
        public:
        /* Commandline/Directory options */

        /* Key Parameters that must be set before
         * RetroShare will start up:
         */
        std::string load_cert;
        std::string load_key;
        std::string passwd;

        bool havePasswd; /* for Commandline password */

        /* Win/Unix Differences */
        char dirSeperator;

        /* Directories */
        std::string basedir;
        std::string homePath;

        /* Listening Port */
        bool forceLocalAddr;
        unsigned short port;
        char inet[256];

        /* Logging */
        bool haveLogFile;
        bool outStderr;
        bool haveDebugLevel;
        int  debugLevel;
        char logfname[1024];

        bool firsttime_run;
        bool load_trustedpeer;
        std::string load_trustedpeer_file;

        bool udpListenerOnly;
};



#endif
