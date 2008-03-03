/*
 * libretroshare/src/pqi: p3dhtmgr.h
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2004-2008 by Robert Fernie.
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


#ifndef MRK_P3_DHT_MANAGER_HEADER
#define MRK_P3_DHT_MANAGER_HEADER

/* Interface class for DHT data */

#include <string>
#include <map>
#include "pqi/pqinetwork.h"

#include "util/rsthreads.h"
#include "pqi/pqimonitor.h"

/* All other #defs are in .cc */
#define DHT_ADDR_INVALID        0xff
#define DHT_ADDR_TCP            0x01
#define DHT_ADDR_UDP            0x02


/* for DHT peer STATE */
#define DHT_PEER_INIT           0
#define DHT_PEER_SEARCH         1
#define DHT_PEER_FOUND          2

/* for DHT peer STATE (ownEntry) */
#define DHT_PEER_ADDR_KNOWN     3
#define DHT_PEER_PUBLISHED      4

/* Interface with Real DHT Implementation */
#define DHT_MODE_SEARCH         1
#define DHT_MODE_PUBLISH        1
#define DHT_MODE_NOTIFY         2



/* TIMEOUTS: Reference Values are set here... */

#define DHT_SEARCH_PERIOD       1800 /* PeerKeys: if we haven't found them: 30 min */
#define DHT_CHECK_PERIOD        1800 /* PeerKeys: re-lookup peer: 30 min */
#define DHT_PUBLISH_PERIOD      1800 /* OwnKey: 30 min */
#define DHT_NOTIFY_PERIOD       300  /* 5 min - Notify Check period */

/* TTLs for DHTs posts */
#define DHT_TTL_PUBLISH         (DHT_PUBLISH_PERIOD + 120)  // for a little overlap.
#define DHT_TTL_NOTIFY          (DHT_NOTIFY_PERIOD  + 60)   // for time to find it...




class dhtPeerEntry
{
        public:
	dhtPeerEntry();

        std::string id;
        uint32_t state;
        time_t lastTS;

	uint32_t notifyPending;
	time_t   notifyTS;

        struct sockaddr_in laddr, raddr;
	uint32_t type;  /* ADDR_TYPE as defined above */

	std::string hash1; /* SHA1 Hash of id */
	std::string hash2; /* SHA1 Hash of reverse Id */
};

class p3DhtMgr: public RsThread
{
	/* 
	 */
	public:
	p3DhtMgr(std::string id, pqiConnectCb *cb);

	/********** External DHT Interface ************************
	 * These Functions are the external interface
	 * for the DHT, and must be non-blocking and return quickly
	 */

void	setDhtOn(bool on);
bool	getDhtOn();
bool	getDhtActive();

	/* set key data */
bool 	setExternalInterface(struct sockaddr_in laddr,
			struct sockaddr_in raddr, uint32_t type);

	/* add / remove peers */
bool 	findPeer(std::string id);
bool 	dropPeer(std::string id);

	/* post DHT key saying we should connect (callback when done) */
bool 	notifyPeer(std::string id); 

	/* extract current peer status */
bool 	getPeerStatus(std::string id, 
			struct sockaddr_in &laddr, struct sockaddr_in &raddr, 
			uint32_t &type, uint32_t &mode);

	/* stun */
bool 	addStun(std::string id);
bool 	doneStun();

	/********** Higher Level DHT Work Functions ************************
	 * These functions translate from the strings/addresss to 
	 * key/value pairs.
	 */
	public:

	/* results from DHT proper */
virtual bool dhtResultNotify(std::string id);
virtual bool dhtResultSearch(std::string id,
		struct sockaddr_in &laddr, struct sockaddr_in &raddr, 
				uint32_t type, std::string sign);

	protected:

	/* can block briefly (called only from thread) */
virtual bool dhtPublish(std::string id,  
		struct sockaddr_in &laddr, 
		struct sockaddr_in &raddr, 
				uint32_t type, std::string sign);

virtual bool dhtNotify(std::string peerid, std::string ownId, 
		std::string sign);
virtual	bool dhtSearch(std::string id, uint32_t mode);


	/********** Actual DHT Work Functions ************************
	 * These involve a very simple LOW-LEVEL interface ... 
	 *
	 * publish
	 * search
	 * result
	 *
	 */

	public:

	/* Feedback callback (handled here) */
virtual bool resultDHT(std::string key, std::string value);

	protected:

virtual bool    init();
virtual bool    shutdown();
virtual bool    dhtActive();
virtual int     status(std::ostream &out);

virtual bool publishDHT(std::string key, std::string value, uint32_t ttl);
virtual	bool searchDHT(std::string key);



	/********** Internal DHT Threading ************************
	 *
	 */

	public:

virtual void run();

	private:

	/* search scheduling */
void 	checkDHTStatus();
int 	checkStunState();
int 	doStun();
int 	checkPeerDHTKeys();
int 	checkOwnDHTKeys();
int 	checkNotifyDHT();

void 	clearDhtData();


	/* other feedback through callback */
	pqiConnectCb *connCb;

	/* protected by Mutex */
	RsMutex dhtMtx;

	bool 	 mDhtOn; /* User desired state */
	bool     mDhtModifications; /* any user requests? */

	dhtPeerEntry ownEntry;
	time_t ownNotifyTS;
	std::map<std::string, dhtPeerEntry> peers;

	std::list<std::string> stunIds;
	bool     mStunRequired;

	uint32_t mDhtState;
	time_t   mDhtActiveTS;

};


#endif // MRK_P3_DHT_MANAGER_HEADER


