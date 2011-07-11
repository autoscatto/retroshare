/*
 * libretroshare/src/pqi: p3peermgr.cc
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2007-2011 by Robert Fernie.
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

#include "util/rsnet.h"
#include "pqi/authgpg.h"
#include "pqi/authssl.h"

#include "pqi/p3peermgr.h"
#include "pqi/p3linkmgr.h"
#include "pqi/p3netmgr.h"

//#include "pqi/p3dhtmgr.h" // Only need it for constants.
//#include "tcponudp/tou.h"
//#include "util/extaddrfinder.h"
//#include "util/dnsresolver.h"

#include "util/rsprint.h"
//#include "util/rsdebug.h"
//const int p3connectzone = 3431;

#include "serialiser/rsconfigitems.h"
#include "pqi/pqinotify.h"
#include "retroshare/rsiface.h"

#include <sstream>

/* Network setup States */

const uint32_t RS_NET_NEEDS_RESET = 	0x0000;
const uint32_t RS_NET_UNKNOWN = 	0x0001;
const uint32_t RS_NET_UPNP_INIT = 	0x0002;
const uint32_t RS_NET_UPNP_SETUP =  	0x0003;
const uint32_t RS_NET_EXT_SETUP =  	0x0004;
const uint32_t RS_NET_DONE =    	0x0005;
const uint32_t RS_NET_LOOPBACK =    	0x0006;
const uint32_t RS_NET_DOWN =    	0x0007;

const uint32_t MIN_TIME_BETWEEN_NET_RESET = 		5;

const uint32_t PEER_IP_CONNECT_STATE_MAX_LIST_SIZE =     	4;

/****
 * #define CONN_DEBUG 1
 * #define CONN_DEBUG_RESET 1
 * #define CONN_DEBUG_TICK 1
 ***/

#define MAX_AVAIL_PERIOD 230 //times a peer stay in available state when not connected
#define MIN_RETRY_PERIOD 140

void  printConnectState(std::ostream &out, peerState &peer);

peerState::peerState()
	:id("unknown"), 
         gpg_id("unknown"),
	 netMode(RS_NET_MODE_UNKNOWN), visState(RS_VIS_STATE_STD), lastcontact(0) 
{
        sockaddr_clear(&localaddr);
        sockaddr_clear(&serveraddr);

	return;
}

std::string textPeerConnectState(peerState &state)
{
	std::ostringstream out;
	out << "Id: " << state.id << std::endl;
	out << "NetMode: " << state.netMode << std::endl;
	out << "VisState: " << state.visState << std::endl;
	out << "laddr: " << rs_inet_ntoa(state.localaddr.sin_addr)
		<< ":" << ntohs(state.localaddr.sin_port) << std::endl;
	out << "eaddr: " << rs_inet_ntoa(state.serveraddr.sin_addr)
		<< ":" << ntohs(state.serveraddr.sin_port) << std::endl;

	std::string output = out.str();
	return output;
}


p3PeerMgr::p3PeerMgr()
	:p3Config(CONFIG_TYPE_PEERS), mPeerMtx("p3PeerMgr"), mStatusChanged(false)
{

	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		/* setup basics of own state */
		mOwnState.id = AuthSSL::getAuthSSL()->OwnId();
		mOwnState.gpg_id = AuthGPG::getAuthGPG()->getGPGOwnId();
		mOwnState.name = AuthGPG::getAuthGPG()->getGPGOwnName();
		mOwnState.location = AuthSSL::getAuthSSL()->getOwnLocation();
		mOwnState.netMode = RS_NET_MODE_UDP;
		// user decided.
		//mOwnState.netMode |= RS_NET_MODE_TRY_UPNP;
	
		lastGroupId = 1;


	}
	
#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr() Startup" << std::endl;
#endif


	return;
}

void    p3PeerMgr::setManagers(p3LinkMgr *linkMgr, p3NetMgr  *netMgr)
{
	mLinkMgr = linkMgr;
	mNetMgr = netMgr;
}

void p3PeerMgr::setOwnNetworkMode(uint32_t netMode)
{
	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

#ifdef CONN_DEBUG
		std::cerr << "p3PeerMgr::setOwnNetworkMode() :";
		std::cerr << " Existing netMode: " << mOwnState.netMode;
		std::cerr << " Input netMode: " << netMode;
		std::cerr << std::endl;
#endif

		mOwnState.netMode = (netMode & RS_NET_MODE_ACTUAL);

		IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
	}
	
	// Pass on Flags to NetMgr.
	mNetMgr->setNetworkMode((netMode & RS_NET_MODE_ACTUAL));
}

void p3PeerMgr::setOwnVisState(uint32_t visState)
{
	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

#ifdef CONN_DEBUG
		std::cerr << "p3PeerMgr::setOwnVisState()";
		std::cerr << "Existing vis: " << mOwnState.visState;
		std::cerr << "Input vis: " << visState;
		std::cerr << std::endl;
#endif

		mOwnState.visState = visState;

		IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
	}
	
	// Pass on Flags to NetMgr.
	mNetMgr->setVisState(visState);
}



void p3PeerMgr::tick()
{

	static time_t last_friends_check = time(NULL) ;
	static const time_t INTERVAL_BETWEEN_LOCATION_CLEANING = 600 ; // Remove unused locations every 10 minutes.

	time_t now = time(NULL) ;

	if(now > last_friends_check + INTERVAL_BETWEEN_LOCATION_CLEANING && rsPeers != NULL)
	{
		std::cerr << "p3PeerMgr::tick(): cleaning unused locations." << std::endl ;

		rsPeers->cleanUnusedLocations() ;
		last_friends_check = now ;
	}
}


/********************************  Network Status  *********************************
 * Configuration Loading / Saving.
 */



const std::string p3PeerMgr::getOwnId()
{
                return AuthSSL::getAuthSSL()->OwnId();
}


bool p3PeerMgr::getOwnNetStatus(peerState &state)
{
        RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
        state = mOwnState;
	return true;
}

bool p3PeerMgr::isFriend(const std::string &id)
{
#ifdef CONN_DEBUG
                std::cerr << "p3PeerMgr::isFriend(" << id << ") called" << std::endl;
#endif
        RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
        bool ret = (mFriendList.end() != mFriendList.find(id));
#ifdef CONN_DEBUG
                std::cerr << "p3PeerMgr::isFriend(" << id << ") returning : " << ret << std::endl;
#endif
        return ret;
}


bool p3PeerMgr::getFriendNetStatus(const std::string &id, peerState &state)
{
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	/* check for existing */
	std::map<std::string, peerState>::iterator it;
	it = mFriendList.find(id);
	if (it == mFriendList.end())
	{
		return false;
	}

	state = it->second;
	return true;
}


bool p3PeerMgr::getOthersNetStatus(const std::string &id, peerState &state)
{
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	/* check for existing */
	std::map<std::string, peerState>::iterator it;
	it = mOthersList.find(id);
	if (it == mOthersList.end())
	{
		return false;
	}

	state = it->second;
	return true;
}


void p3PeerMgr::getFriendList(std::list<std::string> &peers)
{
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	/* check for existing */
        std::map<std::string, peerState>::iterator it;
	for(it = mFriendList.begin(); it != mFriendList.end(); it++)
	{
		peers.push_back(it->first);
	}
	return;
}


#if 0
void p3PeerMgr::getOthersList(std::list<std::string> &peers)
{
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	/* check for existing */
        std::map<std::string, peerState>::iterator it;
	for(it = mOthersList.begin(); it != mOthersList.end(); it++)
	{
		peers.push_back(it->first);
	}
	return;
}
#endif



int p3PeerMgr::getConnectAddresses(const std::string &id, 
					struct sockaddr_in &lAddr, struct sockaddr_in &eAddr, 
					pqiIpAddrSet &histAddrs, std::string &dyndns)
{

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	
	/* check for existing */
	std::map<std::string, peerState>::iterator it;
	it = mFriendList.find(id);
	if (it == mFriendList.end())
	{
		/* ERROR */
		std::cerr << "p3PeerMgr::getConnectAddresses() ERROR unknown Peer";
		std::cerr << std::endl;
		return 0;
	}
	
	lAddr = it->second.localaddr;
	eAddr = it->second.serveraddr;
	histAddrs = it->second.ipAddrs;
	dyndns = it->second.dyndns;

	return 1;
}

/****************************
 * Update state,
 * trigger retry if necessary,
 *
 * remove from DHT?
 *
 */


/******************************** Feedback ......  *********************************
 * From various sources
 */


/*******************************************************************/
/*******************************************************************/

bool p3PeerMgr::addFriend(const std::string &id, const std::string &gpg_id, uint32_t netMode, uint32_t visState, time_t lastContact)
{
	bool notifyLinkMgr = false;
	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/


		if (id == AuthSSL::getAuthSSL()->OwnId()) {
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::addFriend() cannot add own id as a friend." << std::endl;
#endif
			/* (1) already exists */
			return false;
		}
		/* so four possibilities
		 * (1) already exists as friend -> do nothing.
		 * (2) is in others list -> move over.
		 * (3) is non-existant -> create new one.
		 */

#ifdef CONN_DEBUG
		std::cerr << "p3PeerMgr::addFriend() " << id << "; gpg_id : " << gpg_id << std::endl;
#endif

		std::map<std::string, peerState>::iterator it;
		if (mFriendList.end() != mFriendList.find(id))
		{
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::addFriend() Already Exists" << std::endl;
#endif
			/* (1) already exists */
			return true;
		}

		//Authentication is now tested at connection time, we don't store the ssl cert anymore
		//
		if (!AuthGPG::getAuthGPG()->isGPGAccepted(gpg_id) &&  gpg_id != AuthGPG::getAuthGPG()->getGPGOwnId())
		{
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::addFriend() gpg is not accepted" << std::endl;
#endif
			/* no auth */
			return false;
		}


		/* check if it is in others */
		if (mOthersList.end() != (it = mOthersList.find(id)))
		{
			/* (2) in mOthersList -> move over */
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::addFriend() Move from Others" << std::endl;
#endif

			mFriendList[id] = it->second;
			mOthersList.erase(it);

			it = mFriendList.find(id);

			/* setup connectivity parameters */
			it->second.visState = visState;
			it->second.netMode  = netMode;
			it->second.lastcontact = lastContact;

			mStatusChanged = true;

			notifyLinkMgr = true;

			IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
		}
		else
		{
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::addFriend() Creating New Entry" << std::endl;
#endif

			/* create a new entry */
			peerState pstate;

			pstate.id = id;
			pstate.gpg_id = gpg_id;
			pstate.name = AuthGPG::getAuthGPG()->getGPGName(gpg_id);
			
			pstate.visState = visState;
			pstate.netMode = netMode;
			pstate.lastcontact = lastContact;

			/* addr & timestamps -> auto cleared */

			mFriendList[id] = pstate;

			mStatusChanged = true;

			notifyLinkMgr = true;

			IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
		}
	}

	if (notifyLinkMgr)
	{
		mLinkMgr->addFriend(id, !(visState & RS_VIS_STATE_NODHT));
	}

	return true;
}


bool p3PeerMgr::removeFriend(const std::string &id)
{

#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::removeFriend() for id : " << id << std::endl;
	std::cerr << "p3PeerMgr::removeFriend() mFriendList.size() : " << mFriendList.size() << std::endl;
#endif

	mLinkMgr->removeFriend(id);

	std::list<std::string> toRemove;

	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		/* move to othersList */
		bool success = false;
		std::map<std::string, peerState>::iterator it;
		//remove ssl and gpg_ids
		for(it = mFriendList.begin(); it != mFriendList.end(); it++)
		{
			if (it->second.id == id || it->second.gpg_id == id) {
#ifdef CONN_DEBUG
				std::cerr << "p3PeerMgr::removeFriend() friend found in the list." << id << std::endl;
#endif
				peerState peer = it->second;

				toRemove.push_back(it->second.id);

				mOthersList[id] = peer;
				mStatusChanged = true;

				success = true;
			}
		}

		std::list<std::string>::iterator toRemoveIt;
		for(toRemoveIt = toRemove.begin(); toRemoveIt != toRemove.end(); toRemoveIt++) {
			if (mFriendList.end() != (it = mFriendList.find(*toRemoveIt))) {
				mFriendList.erase(it);
			}
		}

#ifdef CONN_DEBUG
		std::cerr << "p3PeerMgr::removeFriend() new mFriendList.size() : " << mFriendList.size() << std::endl;
#endif
	}

	/* remove id from all groups */
	std::list<std::string> peerIds;
	peerIds.push_back(id);

	assignPeersToGroup("", peerIds, false);

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

        return !toRemove.empty();
}



#if 0
bool p3PeerMgr::addNeighbour(std::string id)
{

#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::addNeighbour() not implemented anymore." << id << std::endl;
#endif

	/* so three possibilities 
	 * (1) already exists as friend -> do nothing.
	 * (2) already in others list -> do nothing.
	 * (3) is non-existant -> create new one.
	 */

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

        std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == mFriendList.find(id))
	{
		/* (1) already exists */
		return false;
	}

	if (mOthersList.end() == mOthersList.find(id))
	{
		/* (2) already exists */
		return true;
	}

	/* check with the AuthMgr if its valid */
        if (!AuthSSL::getAuthSSL()->isAuthenticated(id))
	{
		/* no auth */
		return false;
	}

	/* get details from AuthMgr */
        sslcert detail;
        if (!AuthSSL::getAuthSSL()->getCertDetails(id, detail))
	{
		/* no details */
		return false;
	}

	/* create a new entry */
	peerState pstate;

	pstate.id = id;
        pstate.name = detail.name;

	pstate.state = 0;
	pstate.actions = 0; //RS_PEER_NEW;
	pstate.visState = RS_VIS_STATE_STD;
	pstate.netMode = RS_NET_MODE_UNKNOWN;

	/* addr & timestamps -> auto cleared */
	mOthersList[id] = pstate;

        return false;
}

#endif

/*******************************************************************/
/*******************************************************************/


/**********************************************************************
 **********************************************************************
 ******************** External Setup **********************************
 **********************************************************************
 **********************************************************************/


bool    p3PeerMgr::setLocalAddress(const std::string &id, struct sockaddr_in addr)
{
	if (id == AuthSSL::getAuthSSL()->OwnId())
	{
		bool changed = false;
		{
			RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
			if (mOwnState.localaddr.sin_addr.s_addr != addr.sin_addr.s_addr ||
			    mOwnState.localaddr.sin_port != addr.sin_port)
			{
				changed = true;
			}

			mOwnState.localaddr = addr;
		}

		if (changed)
		{
			IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
			
			mNetMgr->setLocalAddress(addr);
			mLinkMgr->setLocalAddress(addr);
			
			#ifdef CONN_DEBUG_RESET
			std::cerr << "p3PeerMgr::setLocalAddress() Calling NetReset" << std::endl;
			#endif
		}
		return true;
	}

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
		if (mOthersList.end() == (it = mOthersList.find(id)))
		{
			#ifdef CONN_DEBUG
					std::cerr << "p3PeerMgr::setLocalAddress() cannot add addres info : peer id not found in friend list  id: " << id << std::endl;
			#endif
			return false;
		}
	}

	/* "it" points to peer */
	it->second.localaddr = addr;

#if 0
	//update ip address list
	IpAddressTimed ipAddressTimed;
	ipAddressTimed.ipAddr = addr;
	ipAddressTimed.seenTime = time(NULL);
	it->second.updateIpAddressList(ipAddressTimed);
#endif

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

	return true;
}

bool    p3PeerMgr::setExtAddress(const std::string &id, struct sockaddr_in addr)
{
	if (id == AuthSSL::getAuthSSL()->OwnId())
	{
		{
			RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
			mOwnState.serveraddr = addr;
		}
		
		mNetMgr->setExtAddress(addr);
		
		return true;
	}

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
		if (mOthersList.end() == (it = mOthersList.find(id)))
		{
#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::setLocalAddress() cannot add addres info : peer id not found in friend list  id: " << id << std::endl;
#endif
			return false;
		}
	}

	/* "it" points to peer */
	it->second.serveraddr = addr;

#if 0
	//update ip address list
	IpAddressTimed ipAddressTimed;
	ipAddressTimed.ipAddr = addr;
	ipAddressTimed.seenTime = time(NULL);
	it->second.updateIpAddressList(ipAddressTimed);
#endif

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

	return true;
}


bool p3PeerMgr::setDynDNS(const std::string &id, const std::string &dyndns)
{
    if (id == AuthSSL::getAuthSSL()->OwnId())
    {
        RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
        mOwnState.dyndns = dyndns;
        return true;
    }

    RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
    /* check if it is a friend */
    std::map<std::string, peerState>::iterator it;
    if (mFriendList.end() == (it = mFriendList.find(id)))
    {
            if (mOthersList.end() == (it = mOthersList.find(id)))
            {
                    #ifdef CONN_DEBUG
                                    std::cerr << "p3PeerMgr::setDynDNS() cannot add dyn dns info : peer id not found in friend list  id: " << id << std::endl;
                    #endif
                    return false;
            }
    }

    /* "it" points to peer */
    it->second.dyndns = dyndns;

    IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

    return true;
}

bool    p3PeerMgr::updateAddressList(const std::string& id, const pqiIpAddrSet &addrs)
{
#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::setAddressList() called for id : " << id << std::endl;
#endif

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	/* check if it is our own ip */
	if (id == getOwnId()) 
	{
		mOwnState.ipAddrs.updateAddrs(addrs);
		return true;
	}

	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
            if (mOthersList.end() == (it = mOthersList.find(id)))
            {
#ifdef CONN_DEBUG
				std::cerr << "p3PeerMgr::setLocalAddress() cannot add addres info : peer id not found in friend list. id: " << id << std::endl;
#endif
                    return false;
            }
	}

	/* "it" points to peer */
	it->second.ipAddrs.updateAddrs(addrs);
#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::setLocalAddress() Updated Address for: " << id;
	std::cerr << std::endl;
	it->second.ipAddrs.printAddrs(std::cerr);
	std::cerr << std::endl;
#endif

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

	return true;
}


bool    p3PeerMgr::updateCurrentAddress(const std::string& id, const pqiIpAddress &addr)
{
#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::updateCurrentAddress() called for id : " << id << std::endl;
#endif
	
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	
	/* cannot be own id */
	
	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
		if (mOthersList.end() == (it = mOthersList.find(id)))
		{
			std::cerr << "p3PeerMgr::updateCurrentAddress() ERROR peer id not found: " << id << std::endl;
			return false;
		}
	}

	if (isPrivateNet(&(addr.mAddr.sin_addr)))
	{
		it->second.ipAddrs.updateLocalAddrs(addr);
		it->second.localaddr = addr.mAddr;
	}
	else
	{
		it->second.ipAddrs.updateExtAddrs(addr);
		it->second.serveraddr = addr.mAddr;
	}

#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::updatedCurrentAddress() Updated Address for: " << id;
	std::cerr << std::endl;
	it->second.ipAddrs.printAddrs(std::cerr);
	std::cerr << std::endl;
#endif
	
	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
	
	return true;
}
	

bool    p3PeerMgr::updateLastContact(const std::string& id)
{
#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::updateLastContact() called for id : " << id << std::endl;
#endif
	
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	
	/* cannot be own id */
	
	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
		if (mOthersList.end() == (it = mOthersList.find(id)))
		{
			std::cerr << "p3PeerMgr::updateLastContact() ERROR peer id not found: " << id << std::endl;
			return false;
		}
	}

	it->second.lastcontact = time(NULL);

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/
	
	return true;
}

	


bool    p3PeerMgr::setNetworkMode(const std::string &id, uint32_t netMode)
{
	if (id == AuthSSL::getAuthSSL()->OwnId())
	{
		setOwnNetworkMode(netMode);
		return true;
	}

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/
	/* check if it is a friend */
	std::map<std::string, peerState>::iterator it;
	if (mFriendList.end() == (it = mFriendList.find(id)))
	{
		if (mOthersList.end() == (it = mOthersList.find(id)))
		{
			return false;
		}
	}

	/* "it" points to peer */
	it->second.netMode = netMode;
	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

	return false;
}

bool    p3PeerMgr::setLocation(const std::string &id, const std::string &location)
{
        RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

#ifdef CONN_DEBUG
        std::cerr << "p3PeerMgr::setLocation() called for id : " << id << "; with location " << location << std::endl;
#endif
        if (id == AuthSSL::getAuthSSL()->OwnId())
        {
                mOwnState.location = location;
                return true;
        }

        /* check if it is a friend */
        std::map<std::string, peerState>::iterator it;
        if (mFriendList.end() == (it = mFriendList.find(id))) {
            return false;
        } else {
            it->second.location = location;
            return true;
        }
}

bool    p3PeerMgr::setVisState(const std::string &id, uint32_t visState)
{
	if (id == AuthSSL::getAuthSSL()->OwnId())
	{
		setOwnVisState(visState);
		return true;
	}

	bool dht_state ;
	bool isFriend = false;
	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		/* check if it is a friend */
		std::map<std::string, peerState>::iterator it;
		if (mFriendList.end() == (it = mFriendList.find(id)))
		{
			if (mOthersList.end() == (it = mOthersList.find(id)))
			{
				return false;
			}
		}
		else
		{
			isFriend = true;
		}

		/* "it" points to peer */
		it->second.visState = visState;
		dht_state = it->second.visState & RS_VIS_STATE_NODHT ;
	}
	if(isFriend)
	{
		/* toggle DHT state */
		if(dht_state)
		{
			/* hidden from DHT world */
			mLinkMgr->setFriendVisibility(id, false);
		}
		else
		{
			mLinkMgr->setFriendVisibility(id, true);
		}
	}

	IndicateConfigChanged(); /**** INDICATE MSG CONFIG CHANGED! *****/

	return false;
}




/*******************************************************************/


/**********************************************************************
 **********************************************************************
 ******************** p3Config functions ******************************
 **********************************************************************
 **********************************************************************/

        /* Key Functions to be overloaded for Full Configuration */

RsSerialiser *p3PeerMgr::setupSerialiser()
{
	RsSerialiser *rss = new RsSerialiser();
	rss->addSerialType(new RsPeerConfigSerialiser());
	rss->addSerialType(new RsGeneralConfigSerialiser()) ;

	return rss;
}


bool p3PeerMgr::saveList(bool &cleanup, std::list<RsItem *>& saveData)
{
	/* create a list of current peers */
	cleanup = false;
	bool useExtAddrFinder = mNetMgr->getIPServersEnabled();
	bool allowTunnelConnection = mLinkMgr->getTunnelConnection();

	mPeerMtx.lock(); /****** MUTEX LOCKED *******/ 

	RsPeerNetItem *item = new RsPeerNetItem();
	item->clear();

	item->pid = getOwnId();
        item->gpg_id = mOwnState.gpg_id;
        item->location = mOwnState.location;
	if (mOwnState.netMode & RS_NET_MODE_TRY_EXT)
	{
		item->netMode = RS_NET_MODE_EXT;
	}
	else if (mOwnState.netMode & RS_NET_MODE_TRY_UPNP)
	{
		item->netMode = RS_NET_MODE_UPNP;
	}
	else
	{
		item->netMode = RS_NET_MODE_UDP;
	}

	item->visState = mOwnState.visState;
	item->lastContact = mOwnState.lastcontact;

        item->currentlocaladdr = mOwnState.localaddr;
        item->currentremoteaddr = mOwnState.serveraddr;
        item->dyndns = mOwnState.dyndns;
        mOwnState.ipAddrs.mLocal.loadTlv(item->localAddrList);
        mOwnState.ipAddrs.mExt.loadTlv(item->extAddrList);

#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::saveList() Own Config Item:" << std::endl;
	item->print(std::cerr, 10);
	std::cerr << std::endl;
#endif

	saveData.push_back(item);
	saveCleanupList.push_back(item);

	/* iterate through all friends and save */
        std::map<std::string, peerState>::iterator it;
	for(it = mFriendList.begin(); it != mFriendList.end(); it++)
	{
		item = new RsPeerNetItem();
		item->clear();

		item->pid = it->first;
		item->gpg_id = (it->second).gpg_id;
		item->location = (it->second).location;
		item->netMode = (it->second).netMode;
		item->visState = (it->second).visState;
		item->lastContact = (it->second).lastcontact;
		item->currentlocaladdr = (it->second).localaddr;
		item->currentremoteaddr = (it->second).serveraddr;
		item->dyndns = (it->second).dyndns;
		(it->second).ipAddrs.mLocal.loadTlv(item->localAddrList);
		(it->second).ipAddrs.mExt.loadTlv(item->extAddrList);

		saveData.push_back(item);
		saveCleanupList.push_back(item);
#ifdef CONN_DEBUG
		std::cerr << "p3PeerMgr::saveList() Peer Config Item:" << std::endl;
		item->print(std::cerr, 10);
		std::cerr << std::endl;
#endif
	}

	// Now save config for network digging strategies
	
	RsConfigKeyValueSet *vitem = new RsConfigKeyValueSet ;

	RsTlvKeyValue kv;
	kv.key = "USE_EXTR_IP_FINDER" ;
	kv.value = (useExtAddrFinder)?"TRUE":"FALSE" ;
	vitem->tlvkvs.pairs.push_back(kv) ;

        #ifdef CONN_DEBUG
	std::cout << "Pushing item for use_extr_addr_finder = " << mUseExtAddrFinder << std::endl ;
        #endif
	saveData.push_back(vitem);
	saveCleanupList.push_back(vitem);

                // Now save config for network digging strategies

        RsConfigKeyValueSet *vitem2 = new RsConfigKeyValueSet ;

        RsTlvKeyValue kv2;
        kv2.key = "ALLOW_TUNNEL_CONNECTION" ;
        kv2.value = (allowTunnelConnection)?"TRUE":"FALSE" ;
        vitem2->tlvkvs.pairs.push_back(kv2) ;

        #ifdef CONN_DEBUG
        std::cout << "Pushing item for allow_tunnel_connection = " << mAllowTunnelConnection << std::endl ;
        #endif
        saveData.push_back(vitem2);
	saveCleanupList.push_back(vitem2);

	/* save groups */

	std::list<RsPeerGroupItem *>::iterator groupIt;
	for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
		saveData.push_back(*groupIt); // no delete
	}

	return true;
}

void    p3PeerMgr::saveDone()
{
	/* clean up the save List */
	std::list<RsItem *>::iterator it;
	for(it = saveCleanupList.begin(); it != saveCleanupList.end(); it++)
	{
		delete (*it);
	}

	saveCleanupList.clear();

	/* unlock mutex */
	mPeerMtx.unlock(); /****** MUTEX UNLOCKED *******/
}

bool  p3PeerMgr::loadList(std::list<RsItem *>& load)
{

	// DEFAULTS.
	bool useExtAddrFinder = true;
	bool allowTunnelConnection = true;
	
        if (load.size() == 0) {
            std::cerr << "p3PeerMgr::loadList() list is empty, it may be a configuration problem."  << std::endl;
            return false;
        }

#ifdef CONN_DEBUG
	std::cerr << "p3PeerMgr::loadList() Item Count: " << load.size() << std::endl;
#endif

	std::string ownId = getOwnId();

	/* load the list of peers */
	std::list<RsItem *>::iterator it;
	for(it = load.begin(); it != load.end(); it++)
	{
		RsPeerNetItem *pitem = dynamic_cast<RsPeerNetItem *>(*it);
		if (pitem)
		{
			if (pitem->pid == ownId)
			{
#ifdef CONN_DEBUG
				std::cerr << "p3PeerMgr::loadList() Own Config Item:" << std::endl;
				pitem->print(std::cerr, 10);
				std::cerr << std::endl;
#endif
				/* add ownConfig */
				setOwnNetworkMode(pitem->netMode);
				setOwnVisState(pitem->visState);
				
				mOwnState.gpg_id = AuthGPG::getAuthGPG()->getGPGOwnId();
				mOwnState.location = AuthSSL::getAuthSSL()->getOwnLocation();
			}
			else
			{
#ifdef CONN_DEBUG
				std::cerr << "p3PeerMgr::loadList() Peer Config Item:" << std::endl;
				pitem->print(std::cerr, 10);
				std::cerr << std::endl;
#endif
				/* ************* */
				addFriend(pitem->pid, pitem->gpg_id, pitem->netMode, pitem->visState, pitem->lastContact);
				setLocation(pitem->pid, pitem->location);
			}
			
			setLocalAddress(pitem->pid, pitem->currentlocaladdr);
			setExtAddress(pitem->pid, pitem->currentremoteaddr);
			setDynDNS (pitem->pid, pitem->dyndns);

			/* convert addresses */
			pqiIpAddrSet addrs;
			addrs.mLocal.extractFromTlv(pitem->localAddrList);
			addrs.mExt.extractFromTlv(pitem->extAddrList);
			
			updateAddressList(pitem->pid, addrs);

			delete(*it);

			continue;
		}

		RsConfigKeyValueSet *vitem = dynamic_cast<RsConfigKeyValueSet *>(*it) ;
		if (vitem)
		{
			RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::loadList() General Variable Config Item:" << std::endl;
			vitem->print(std::cerr, 10);
			std::cerr << std::endl;
#endif
			std::list<RsTlvKeyValue>::iterator kit;
			for(kit = vitem->tlvkvs.pairs.begin(); kit != vitem->tlvkvs.pairs.end(); kit++) {
				if(kit->key == "USE_EXTR_IP_FINDER") {
					useExtAddrFinder = (kit->value == "TRUE");
					std::cerr << "setting use_extr_addr_finder to " << useExtAddrFinder << std::endl ;
				} else if (kit->key == "ALLOW_TUNNEL_CONNECTION") {
					allowTunnelConnection = (kit->value == "TRUE");
					std::cerr << "setting allow_tunnel_connection to " << allowTunnelConnection << std::endl ;
				}
			}

			delete(*it);

			continue;
		}

		RsPeerGroupItem *gitem = dynamic_cast<RsPeerGroupItem *>(*it) ;
		if (gitem)
		{
			RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

#ifdef CONN_DEBUG
			std::cerr << "p3PeerMgr::loadList() Peer group item:" << std::endl;
			gitem->print(std::cerr, 10);
			std::cerr << std::endl;
#endif

			groupList.push_back(gitem); // don't delete

			if ((gitem->flag & RS_GROUP_FLAG_STANDARD) == 0) {
				/* calculate group id */
				uint32_t groupId = atoi(gitem->id.c_str());
				if (groupId > lastGroupId) {
					lastGroupId = groupId;
				}
			}

			continue;
		}

		delete (*it);
	}

	{
		/* set missing groupIds */

		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		/* Standard groups */
		const int standardGroupCount = 5;
		const char *standardGroup[standardGroupCount] = { RS_GROUP_ID_FRIENDS, RS_GROUP_ID_FAMILY, RS_GROUP_ID_COWORKERS, RS_GROUP_ID_OTHERS, RS_GROUP_ID_FAVORITES };
		bool foundStandardGroup[standardGroupCount] = { false, false, false, false, false };

		std::list<RsPeerGroupItem *>::iterator groupIt;
		for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
			if ((*groupIt)->flag & RS_GROUP_FLAG_STANDARD) {
				int i;
				for (i = 0; i < standardGroupCount; i++) {
					if ((*groupIt)->id == standardGroup[i]) {
						foundStandardGroup[i] = true;
						break;
					}
				}
				
				if (i >= standardGroupCount) {
					/* No more a standard group, remove the flag standard */
					(*groupIt)->flag &= ~RS_GROUP_FLAG_STANDARD;
				}
			} else {
				uint32_t groupId = atoi((*groupIt)->id.c_str());
				if (groupId == 0) {
					std::ostringstream out;
					out << (lastGroupId++);
					(*groupIt)->id = out.str();
				}
			}
		}
		
		/* Initialize standard groups */
		for (int i = 0; i < standardGroupCount; i++) {
			if (foundStandardGroup[i] == false) {
				RsPeerGroupItem *gitem = new RsPeerGroupItem;
				gitem->id = standardGroup[i];
				gitem->name = standardGroup[i];
				gitem->flag |= RS_GROUP_FLAG_STANDARD;
				groupList.push_back(gitem);
			}
		}
	}

	mNetMgr->setIPServersEnabled(useExtAddrFinder);
	mLinkMgr->setTunnelConnection(allowTunnelConnection);
	
	return true;
}


#if 0

void  printConnectState(std::ostream &out, peerState &peer)
{

	out << "Friend: " << peer.name << " Id: " << peer.id << " State: " << peer.state;
	if (peer.state & RS_PEER_S_FRIEND)
		out << " S:RS_PEER_S_FRIEND";
	if (peer.state & RS_PEER_S_ONLINE)
		out << " S:RS_PEER_S_ONLINE";
	if (peer.state & RS_PEER_S_CONNECTED)
		out << " S:RS_PEER_S_CONNECTED";
	out << " Actions: " << peer.actions;
	if (peer.actions & RS_PEER_NEW)
		out << " A:RS_PEER_NEW";
	if (peer.actions & RS_PEER_MOVED)
		out << " A:RS_PEER_MOVED";
	if (peer.actions & RS_PEER_CONNECTED)
		out << " A:RS_PEER_CONNECTED";
	if (peer.actions & RS_PEER_DISCONNECTED)
		out << " A:RS_PEER_DISCONNECTED";
	if (peer.actions & RS_PEER_CONNECT_REQ)
		out << " A:RS_PEER_CONNECT_REQ";

	out << std::endl;
	return;
}

#endif


/**********************************************************************
 **********************************************************************
 ************************** Groups ************************************
 **********************************************************************
 **********************************************************************/

bool p3PeerMgr::addGroup(RsGroupInfo &groupInfo)
{
	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		RsPeerGroupItem *groupItem = new RsPeerGroupItem;
		groupItem->set(groupInfo);

		std::ostringstream out;
		out << (++lastGroupId);
		groupItem->id = out.str();

		// remove standard flag
		groupItem->flag &= ~RS_GROUP_FLAG_STANDARD;

		groupItem->PeerId(getOwnId());

		groupList.push_back(groupItem);
	}

	rsicontrol->getNotify().notifyListChange(NOTIFY_LIST_GROUPLIST, NOTIFY_TYPE_ADD);

	IndicateConfigChanged();

	return true;
}

bool p3PeerMgr::editGroup(const std::string &groupId, RsGroupInfo &groupInfo)
{
	if (groupId.empty()) {
		return false;
	}

	bool changed = false;

	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		std::list<RsPeerGroupItem*>::iterator groupIt;
		for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
			if ((*groupIt)->id == groupId) {
				break;
			}
		}

		if (groupIt != groupList.end()) {
			if ((*groupIt)->flag & RS_GROUP_FLAG_STANDARD) {
				// can't edit standard groups
			} else {
				changed = true;
				(*groupIt)->set(groupInfo);
			}
		}
	}

	if (changed) {
		rsicontrol->getNotify().notifyListChange(NOTIFY_LIST_GROUPLIST, NOTIFY_TYPE_MOD);

		IndicateConfigChanged();
	}

	return changed;
}

bool p3PeerMgr::removeGroup(const std::string &groupId)
{
	if (groupId.empty()) {
		return false;
	}

	bool changed = false;

	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		std::list<RsPeerGroupItem*>::iterator groupIt;
		for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
			if ((*groupIt)->id == groupId) {
				break;
			}
		}

		if (groupIt != groupList.end()) {
			if ((*groupIt)->flag & RS_GROUP_FLAG_STANDARD) {
				// can't remove standard groups
			} else {
				changed = true;
				delete(*groupIt);
				groupList.erase(groupIt);
			}
		}
	}

	if (changed) {
		rsicontrol->getNotify().notifyListChange(NOTIFY_LIST_GROUPLIST, NOTIFY_TYPE_DEL);

		IndicateConfigChanged();
	}

	return changed;
}

bool p3PeerMgr::getGroupInfo(const std::string &groupId, RsGroupInfo &groupInfo)
{
	if (groupId.empty()) {
		return false;
	}

	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	std::list<RsPeerGroupItem*>::iterator groupIt;
	for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
		if ((*groupIt)->id == groupId) {
			(*groupIt)->get(groupInfo);

			return true;
		}
	}

	return false;
}

bool p3PeerMgr::getGroupInfoList(std::list<RsGroupInfo> &groupInfoList)
{
	RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

	std::list<RsPeerGroupItem*>::iterator groupIt;
	for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
		RsGroupInfo groupInfo;
		(*groupIt)->get(groupInfo);
		groupInfoList.push_back(groupInfo);
	}

	return true;
}

// groupId == "" && assign == false -> remove from all groups
bool p3PeerMgr::assignPeersToGroup(const std::string &groupId, const std::list<std::string> &peerIds, bool assign)
{
	if (groupId.empty() && assign == true) {
		return false;
	}

	if (peerIds.empty()) {
		return false;
	}

	bool changed = false;

	{
		RsStackMutex stack(mPeerMtx); /****** STACK LOCK MUTEX *******/

		std::list<RsPeerGroupItem*>::iterator groupIt;
		for (groupIt = groupList.begin(); groupIt != groupList.end(); groupIt++) {
			if (groupId.empty() || (*groupIt)->id == groupId) {
				RsPeerGroupItem *groupItem = *groupIt;

				std::list<std::string>::const_iterator peerIt;
				for (peerIt = peerIds.begin(); peerIt != peerIds.end(); peerIt++) {
					std::list<std::string>::iterator peerIt1 = std::find(groupItem->peerIds.begin(), groupItem->peerIds.end(), *peerIt);
					if (assign) {
						if (peerIt1 == groupItem->peerIds.end()) {
							groupItem->peerIds.push_back(*peerIt);
							changed = true;
						}
					} else {
						if (peerIt1 != groupItem->peerIds.end()) {
							groupItem->peerIds.erase(peerIt1);
							changed = true;
						}
					}
				}

				if (groupId.empty() == false) {
					break;
				}
			}
		}
	}

	if (changed) {
		rsicontrol->getNotify().notifyListChange(NOTIFY_LIST_GROUPLIST, NOTIFY_TYPE_MOD);

		IndicateConfigChanged();
	}

	return changed;
}