/*
 * libretroshare/src/dht: p3bitdht.h
 *
 * BitDht interface for RetroShare.
 *
 * Copyright 2009-2010 by Robert Fernie.
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


#ifndef MRK_P3_BITDHT_H
#define MRK_P3_BITDHT_H

#include "pqi/pqiassist.h"
#include "pqi/p3cfgmgr.h"
#include "retroshare/rsdht.h"

#include <string>
#include <map>
#include "pqi/pqinetwork.h"
#include "pqi/pqimonitor.h"
#include "util/rsthreads.h"

#include "udp/udpstack.h"
#include "udp/udpbitdht.h"
#include "bitdht/bdiface.h"

#include "dht/connectstatebox.h"

class DhtPeerDetails
{
	public:

	DhtPeerDetails();

	uint32_t mPeerType;

	bdId    mDhtId;
	std::string mRsId;

	/* direct from the DHT! */
	uint32_t mDhtState; // One of RSDHT_PEERDHT_[...]
	time_t   mDhtUpdateTS;

	/* internal state */
	PeerConnectStateBox mConnectLogic;
	
	/* Actual Connection Status */
	uint32_t  		mPeerConnectState; // One of RSDHT_PEERCONN_
	std::string 		mPeerConnectMsg;
	uint32_t 		mPeerConnectMode;
	bdId 			mPeerConnectPeerId;
	bdId 			mPeerConnectProxyId;
	struct sockaddr_in 	mPeerConnectAddr;
	uint32_t 		mPeerConnectPoint;
	
	time_t 		mPeerConnectUdpTS;
	time_t 		mPeerConnectTS;
	time_t		mPeerConnectClosedTS;
	
	bool 			mExclusiveProxyLock;

	/* keeping the PeerCbMsg, as we will need it for debugging */
	/* don't think this data is ever used for decisions??? */

        /* Connection Request Status */
	std::string		mPeerReqStatusMsg;
	uint32_t		mPeerReqState;
	uint32_t		mPeerReqMode;
	bdId			mPeerReqProxyId;
	time_t			mPeerReqTS;

        /* Callback Info */
	std::string		mPeerCbMsg;
	uint32_t		mPeerCbMode;
	uint32_t		mPeerCbPoint;
	bdId			mPeerCbProxyId;
	bdId			mPeerCbDestId;
	time_t			mPeerCbTS;

};

#define PEERNET_ACTION_TYPE_CONNECT     1
#define PEERNET_ACTION_TYPE_AUTHORISE   2
#define PEERNET_ACTION_TYPE_START       3
#define PEERNET_ACTION_TYPE_RESTARTREQ  4
#define PEERNET_ACTION_TYPE_KILLREQ     5
#define PEERNET_ACTION_TYPE_TCPATTEMPT  6

class PeerAction
{
        public:

        uint32_t mType;
        bdId mSrcId;
        bdId mProxyId;
        bdId mDestId;
        uint32_t mMode;
        uint32_t mPoint;
        uint32_t mAnswer;
        uint32_t mDelayOrBandwidth;
};



/****** 
 * Adding the ability to install alternative Handler
 * for monitoring/controlling Relay Connections outside of p3bitdht.
 ***/

class p3BitDhtRelayHandler
{
	public:

	int (*mInstallRelay)(const bdId *srcId, const bdId *destId, uint32_t mode, uint32_t &bandwidth);
	int (*mLogFailedConnection)(const bdId *srcId, const bdId *destId, uint32_t mode, uint32_t errcode);
};




class UdpRelayReceiver;
class UdpStunner;
class p3NetMgr;

class p3BitDht: public p3Config, public pqiNetAssistConnect, public RsDht
{
	public:
	p3BitDht(std::string id, pqiConnectCb *cb, p3NetMgr *nm,
		UdpStack *udpstack, std::string bootstrapfile);


virtual	~p3BitDht();


/***********************************************************************************************
 ********** External RsDHT Interface (defined in libretroshare/src/retroshare/rsdht.h) *********
************************************************************************************************/

virtual uint32_t getNetState(uint32_t type);
virtual int      getDhtPeers(int lvl, std::list<RsDhtPeer> &peers);
virtual int      getNetPeerList(std::list<std::string> &peerIds);
virtual int      getNetPeerStatus(std::string peerId, RsDhtNetPeer &status);

virtual int     getRelayEnds(std::list<RsDhtRelayEnd> &relayEnds);
virtual int     getRelayProxies(std::list<RsDhtRelayProxy> &relayProxies);

//virtual int      getNetFailedPeer(std::string peerId, PeerStatus &status);

virtual std::string getUdpAddressString();

virtual void    getDhtRates(float &read, float &write);
virtual void    getRelayRates(float &read, float &write, float &relay);

virtual bool    getOwnDhtId(std::string &ownDhtId);

/***********************************************************************************************
 ********** External RsDHT Interface (defined in libretroshare/src/retroshare/rsdht.h) *********
************************************************************************************************/


	void	setupConnectBits(UdpStunner *dhtStunner, UdpStunner *proxyStunner, UdpRelayReceiver  *relay);
	void	setupPeerSharer(pqiNetAssistPeerShare *sharer);
	void    modifyNodesPerBucket(uint16_t count);

void	start(); /* starts up the bitdht thread */

	/* pqiNetAssist - external interface functions */
virtual int     tick();
virtual void    enable(bool on);  
virtual void    shutdown(); /* blocking call */
virtual void	restart();

virtual bool    getEnabled();
virtual bool    getActive();
virtual bool    getNetworkStats(uint32_t &netsize, uint32_t &localnetsize);

	/* pqiNetAssistConnect - external interface functions */

	/* add / remove peers */
virtual bool 	findPeer(std::string id);
virtual bool 	dropPeer(std::string id);

virtual int addBadPeer(const struct sockaddr_in &addr, uint32_t reason, uint32_t flags, uint32_t age);
virtual int addKnownPeer(const std::string &pid, const struct sockaddr_in &addr, uint32_t flags);
//virtual int 	addFriend(const std::string pid);
//virtual int 	addFriendOfFriend(const std::string pid);
//virtual int 	addOther(const std::string pid);

	/* feedback on success failure of Connections */
virtual void 	ConnectionFeedback(std::string pid, int state);

	/* extract current peer status */
virtual bool 	getPeerStatus(std::string id, 
			struct sockaddr_in &laddr, struct sockaddr_in &raddr, 
					uint32_t &type, uint32_t &mode);

virtual bool 	getExternalInterface(struct sockaddr_in &raddr, 
					uint32_t &mode);


virtual bool    setAttachMode(bool on);



	/* notifyPeer/setExtInterface/Bootstrap/Stun 
	 * hould all be removed from NetAssist?
	 */



	/* pqiNetAssistConnect - external interface functions */


/***********************************************************************************************
 ****************************** Connections (p3bitdht_peernet.cc) ******************************
************************************************************************************************/
	/* Feedback from RS Upper Layers */
//virtual void 	ConnectionFeedback(std::string pid, int state);

	/* Callback functions - from bitdht */
int 	NodeCallback(const bdId *id, uint32_t peerflags);
int 	PeerCallback(const bdId *id, uint32_t status);
int 	ValueCallback(const bdNodeId *id, std::string key, uint32_t status);
int 	ConnectCallback(const bdId *srcId, const bdId *proxyId, const bdId *destId,
				uint32_t mode, uint32_t point, uint32_t param, uint32_t cbtype, uint32_t errcode);
int 	InfoCallback(const bdId *id, uint32_t type, uint32_t flags, std::string info);


int 	OnlinePeerCallback_locked(const bdId *id, uint32_t status, DhtPeerDetails *dpd);
int 	UnreachablePeerCallback_locked(const bdId *id, uint32_t status, DhtPeerDetails *dpd);
//int 	tick();
int 	minuteTick();
int 	doActions();
int 	checkProxyAllowed(const bdId *srcId, const bdId *destId, int mode, uint32_t &bandwidth);
int 	checkConnectionAllowed(const bdId *peerId, int mode);
void 	initiateConnection(const bdId *srcId, const bdId *proxyId, const bdId *destId, uint32_t mode, uint32_t loc, uint32_t delayOrBandwidth);
int 	installRelayConnection(const bdId *srcId, const bdId *destId, uint32_t &bandwidth);
int 	removeRelayConnection(const bdId *srcId, const bdId *destId);
void 	monitorConnections();

void    ConnectCallout(const std::string &peerId, struct sockaddr_in addr, uint32_t connectMode);

void 	ConnectCalloutTCPAttempt(const std::string &peerId, struct sockaddr_in addr);
void 	ConnectCalloutDirectOrProxy(const std::string &peerId, struct sockaddr_in raddr, uint32_t connectFlags, uint32_t delay);
void 	ConnectCalloutRelay(const std::string &peerId, struct sockaddr_in srcaddr, 
			struct sockaddr_in proxyaddr, struct sockaddr_in destaddr,
                        uint32_t connectMode, uint32_t bandwidth);


void 	Feedback_Connected(std::string pid);
void 	Feedback_ConnectionFailed(std::string pid);
void 	Feedback_ConnectionClosed(std::string pid);

void 	UdpConnectionFailed_locked(DhtPeerDetails *dpd);
void 	ReleaseProxyExclusiveMode_locked(DhtPeerDetails *dpd, bool addrChgLikely);


	/*** RELAY HANDLER CODE ***/
void 	installRelayHandler(p3BitDhtRelayHandler *);
UdpRelayReceiver *getRelayReceiver();

int 	RelayHandler_InstallRelayConnection(const bdId *srcId, const bdId *destId, uint32_t mode, uint32_t &bandwidth);
int 	RelayHandler_LogFailedProxyAttempt(const bdId *srcId, const bdId *destId, uint32_t mode, uint32_t errcode);



/***********************************************************************************************
 ******************** Relay Config Stuff (TEMP - MOSTLY, in p3bitdht_relay.cc) *****************
 ********** External RsDHT Interface (defined in libretroshare/src/retroshare/rsdht.h) *********
************************************************************************************************/

        // Interface for controlling Relays & DHT Relay Mode
virtual int     getRelayServerList(std::list<std::string> &ids);
virtual int     addRelayServer(std::string ids);
virtual int     removeRelayServer(std::string ids);

virtual uint32_t getRelayMode();
virtual int      setRelayMode(uint32_t mode);

virtual int     getRelayAllowance(int  classIdx, uint32_t &count, uint32_t &bandwidth);
virtual int     setRelayAllowance(int classIdx, uint32_t  count, uint32_t  bandwidth);

	private:

	// Relay Handling Code / Variables (Mutex Protected).
int 	setupRelayDefaults();
int     pushRelayServers();

	std::list<std::string> mRelayServerList;
	uint32_t mRelayMode;

        protected:
/*****************************************************************/
/***********************  p3config  ******************************/
        /* Key Functions to be overloaded for Full Configuration */
        virtual RsSerialiser *setupSerialiser();
        virtual bool saveList(bool &cleanup, std::list<RsItem *>&);
        virtual void saveDone();
        virtual bool    loadList(std::list<RsItem *>& load);
/*****************************************************************/

	// DATA RATES: Variables (Mutex Protected).
	private:

	void 	updateDataRates();
	void	clearDataRates();

	float mRelayReadRate;
	float mRelayWriteRate;
	float mRelayRelayRate;
	float mDhtReadRate;
	float mDhtWriteRate;

	time_t mLastDataRateUpdate;


/***********************************************************************************************
 ************************** Internal Accounting (p3bitdht_peers.cc) ****************************
************************************************************************************************/

	public:

int 	removePeer(const std::string pid);

	// Can be used externally too.
int 	calculateNodeId(const std::string pid, bdNodeId *id);
int 	addKnownNode(const bdId *id, uint32_t flags);

	private:

DhtPeerDetails *addInternalPeer_locked(const std::string pid, uint32_t type);
int 	removeInternalPeer_locked(const std::string pid);
DhtPeerDetails *findInternalDhtPeer_locked(const bdNodeId *id, uint32_t type);
DhtPeerDetails *findInternalRsPeer_locked(const std::string &pid);

bool 	havePeerTranslation_locked(const std::string &pid);
int 	lookupNodeId_locked(const std::string pid, bdNodeId *id);
int 	lookupRsId_locked(const bdNodeId *id, std::string &pid);
int 	storeTranslation_locked(const std::string pid);
int 	removeTranslation_locked(const std::string pid);

	UdpBitDht *mUdpBitDht; /* has own mutex, is static except for creation/destruction */
	UdpStunner *mDhtStunner;
	UdpStunner *mProxyStunner;
	UdpRelayReceiver   *mRelay;

	p3NetMgr *mNetMgr;

	pqiNetAssistPeerShare *mPeerSharer;

        bdDhtFunctions *mDhtFns;

	RsMutex dhtMtx;


	p3BitDhtRelayHandler *mRelayHandler;

	std::string mOwnRsId;
	bdNodeId    mOwnDhtId;

	time_t mMinuteTS;

	/* translation maps */
        std::map<std::string, bdNodeId> mTransToNodeId;
        std::map<bdNodeId, std::string> mTransToRsId;

	std::map<bdNodeId, DhtPeerDetails> mPeers;
	std::map<bdNodeId, DhtPeerDetails> mFailedPeers;

	/* Connection Action Queue */
	std::list<PeerAction> mActions;

};

#endif /* MRK_P3_BITDHT_H */

