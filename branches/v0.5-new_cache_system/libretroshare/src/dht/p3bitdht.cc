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


#include "dht/p3bitdht.h"

#include "bitdht/bdstddht.h"

#include "tcponudp/udprelay.h"
#include "tcponudp/udpstunner.h"

#include <openssl/sha.h>


/* This is a conversion callback class between pqi interface
 * and the BitDht Interface.
 *
 */

/**** EXTERNAL INTERFACE DHT POINTER *****/
RsDht *rsDht = NULL;

class p3BdCallback: public BitDhtCallback
{
	public:

	p3BdCallback(p3BitDht *parent)
	:mParent(parent) { return; }

virtual int dhtNodeCallback(const bdId *id, uint32_t peerflags)
{
	return mParent->NodeCallback(id, peerflags);
}

virtual int dhtPeerCallback(const bdId *id, uint32_t status)
{
	return mParent->PeerCallback(id, status);
}

virtual int dhtValueCallback(const bdNodeId *id, std::string key, uint32_t status)
{
	return mParent->ValueCallback(id, key, status);
}

virtual int dhtConnectCallback(const bdId *srcId, const bdId *proxyId, const bdId *destId,
                        uint32_t mode, uint32_t point, uint32_t param, uint32_t cbtype, uint32_t errcode)
{ 
	return mParent->ConnectCallback(srcId, proxyId, destId, mode, point, param, cbtype, errcode);
}  

virtual int dhtInfoCallback(const bdId *id, uint32_t type, uint32_t flags, std::string info)
{ 
	return mParent->InfoCallback(id, type, flags, info);
}  

	private:

	p3BitDht *mParent;
};


p3BitDht::p3BitDht(std::string id, pqiConnectCb *cb, p3NetMgr *nm, 
			UdpStack *udpstack, std::string bootstrapfile)
	:p3Config(CONFIG_TYPE_BITDHT), pqiNetAssistConnect(id, cb), mNetMgr(nm), dhtMtx("p3BitDht")
{
	mDhtStunner = NULL;
	mProxyStunner = NULL;
	mRelay = NULL;

        mPeerSharer = NULL;

	mRelayHandler = NULL;

	std::string dhtVersion = "RS51"; // should come from elsewhere!
        mOwnRsId = id;

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::p3BitDht()" << std::endl;
	std::cerr << "Using Id: " << id;
	std::cerr << std::endl;
	std::cerr << "Using Bootstrap File: " << bootstrapfile;
	std::cerr << std::endl;
	std::cerr << "Converting OwnId to bdNodeId....";
	std::cerr << std::endl;
#endif

	/* setup ownId */
	storeTranslation_locked(id);
	lookupNodeId_locked(id, &mOwnDhtId);


#ifdef DEBUG_BITDHT
	std::cerr << "Own NodeId: ";
	bdStdPrintNodeId(std::cerr, &mOwnDhtId);
	std::cerr << std::endl;
#endif

	/* standard dht behaviour */
	bdDhtFunctions *stdfns = new bdStdDht();

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht() startup ... creating UdpBitDht";
	std::cerr << std::endl;
#endif

	/* create dht */
	mUdpBitDht = new UdpBitDht(udpstack, &mOwnDhtId, dhtVersion, bootstrapfile, stdfns);
	udpstack->addReceiver(mUdpBitDht);

	/* setup callback to here */
	p3BdCallback *bdcb = new p3BdCallback(this);
	mUdpBitDht->addCallback(bdcb);

#if 0
	/* enable all modes */
	/* Switched to only Proxy Mode - as Direct Connections can be unreliable - as they share the UDP with the DHT....
	 * We'll get these working properly and then if necessary get Direct further tested.
	 */
	mUdpBitDht->ConnectionOptions(
			// BITDHT_CONNECT_MODE_DIRECT | BITDHT_CONNECT_MODE_PROXY | BITDHT_CONNECT_MODE_RELAY,
			//BITDHT_CONNECT_MODE_DIRECT | BITDHT_CONNECT_MODE_PROXY,
			BITDHT_CONNECT_MODE_PROXY,
                        BITDHT_CONNECT_OPTION_AUTOPROXY);

#endif

	setupRelayDefaults();
}

p3BitDht::~p3BitDht()
{
	//udpstack->removeReceiver(mUdpBitDht);
	delete mUdpBitDht;
}


void    p3BitDht::setupConnectBits(UdpStunner *dhtStunner, UdpStunner *proxyStunner, UdpRelayReceiver  *relay)
{
	mDhtStunner = dhtStunner;
	mProxyStunner = proxyStunner;
	mRelay = relay;
}

void    p3BitDht::setupPeerSharer(pqiNetAssistPeerShare *sharer)
{
	mPeerSharer = sharer;
}


/* Support for Outsourced Relay Handling */

void    p3BitDht::installRelayHandler(p3BitDhtRelayHandler *handler)
{
	/* The Handler is mutex protected, as its installation can occur when the dht is already running */
	RsStackMutex stack(dhtMtx);    /********* LOCKED *********/

	mRelayHandler = handler;
}

UdpRelayReceiver *p3BitDht::getRelayReceiver()
{
	return mRelay;
}


void    p3BitDht::start()
{
#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::start()";
	std::cerr << std::endl;
#endif

	mUdpBitDht->start(); /* starts up the bitdht thread */

	/* dht switched on by config later. */
}

	/* pqiNetAssist - external interface functions */
void    p3BitDht::enable(bool on)
{
#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::enable(" << on << ")";
	std::cerr << std::endl;
#endif

	if (on)
	{
		mUdpBitDht->startDht();
	}
	else
	{
		mUdpBitDht->stopDht();
	}
}
	
void    p3BitDht::shutdown() /* blocking call */
{
	mUdpBitDht->stopDht();
}


void	p3BitDht::restart()
{
	mUdpBitDht->stopDht();
	mUdpBitDht->startDht();
}

bool    p3BitDht::getEnabled()
{
	return (mUdpBitDht->stateDht() != 0);
}

bool    p3BitDht::getActive()
{
	return (mUdpBitDht->stateDht() >= BITDHT_MGR_STATE_ACTIVE);
}

bool    p3BitDht::getNetworkStats(uint32_t &netsize, uint32_t &localnetsize)
{
	netsize = mUdpBitDht->statsNetworkSize();
	localnetsize = mUdpBitDht->statsBDVersionSize();
	return true;
}

#if 0
	/* pqiNetAssistConnect - external interface functions */
	/* add / remove peers */
bool 	p3BitDht::findPeer(std::string pid)
{
#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::findPeer(" << pid << ")";
	std::cerr << std::endl;
#endif

	/* convert id -> NodeId */
	if (!storeTranslation(pid))
	{
		std::cerr << "p3BitDht::findPeer() Failed to storeTranslation";
		std::cerr << std::endl;

		/* error */
		return false;
	}

	bdNodeId nid;
	if (!lookupNodeId(pid, &nid))
	{
		std::cerr << "p3BitDht::findPeer() Failed to lookupNodeId";
		std::cerr << std::endl;

		/* error */
		return false;
	}

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::findPeer() calling AddFindNode() with pid => NodeId: ";
	bdStdPrintNodeId(std::cerr, &nid);
	std::cerr << std::endl;
#endif

	/* add in peer */
	mUdpBitDht->addFindNode(&nid, BITDHT_QFLAGS_DO_IDLE);

	return true ;
}

bool 	p3BitDht::dropPeer(std::string pid)
{
#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::dropPeer(" << pid << ")";
	std::cerr << std::endl;
#endif

	/* convert id -> NodeId */
	bdNodeId nid;
	if (!lookupNodeId(pid, &nid))
	{
		std::cerr << "p3BitDht::dropPeer() Failed to lookup NodeId";
		std::cerr << std::endl;

		/* error */
		return false;
	}

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::dropPeer() Translated to NodeId: ";
	bdStdPrintNodeId(std::cerr, &nid);
	std::cerr << std::endl;
#endif

	/* remove in peer */
	mUdpBitDht->removeFindNode(&nid);

	/* remove from translation */
	if (!removeTranslation(pid))
	{
		std::cerr << "p3BitDht::dropPeer() Failed to removeTranslation";
		std::cerr << std::endl;

		/* error */
		return false;
	}

	return true ;
}

#endif


	/* extract current peer status */
bool 	p3BitDht::getPeerStatus(std::string id, 
				struct sockaddr_in &/*laddr*/, struct sockaddr_in &/*raddr*/,
				uint32_t &/*type*/, uint32_t &/*mode*/)
{
	/* remove unused parameter warnings */
	(void) id;

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::getPeerStatus(" << id << ")";
	std::cerr << std::endl;
#endif

	return false;
}

bool 	p3BitDht::getExternalInterface(struct sockaddr_in &/*raddr*/,
					uint32_t &/*mode*/)
{

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::getExternalInterface()";
	std::cerr << std::endl;
#endif


	return false;
}


bool 	p3BitDht::setAttachMode(bool on)
{

#ifdef DEBUG_BITDHT
	std::cerr << "p3BitDht::setAttachMode(" << on << ")";
	std::cerr << std::endl;
#endif

	return mUdpBitDht->setAttachMode(on);
}




