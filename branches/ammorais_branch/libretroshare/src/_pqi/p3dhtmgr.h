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


#ifndef P3DHDHTMGR_H
#define P3DHDHTMGR_H

/* Interface class for DHT data */

#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <openssl/sha.h>

#include "_pqi/pqinetwork.h"

#include "_util/rsthreads.h"
#include "_pqi/pqimonitor.h"

#include "_pqi/pqiassist.h"




#include "_pqi/p3connmgr.h"

#include "_util/rsprint.h"
#include "_util/rsdebug.h"

/* All other #defs are in .cc */
#define DHT_ADDR_INVALID        0xff
#define DHT_ADDR_TCP            0x01
#define DHT_ADDR_UDP            0x02


/* for DHT peer STATE */
#define DHT_PEER_OFF            0
#define DHT_PEER_INIT           1
#define DHT_PEER_SEARCH         2
#define DHT_PEER_FOUND          3

/* for DHT peer STATE (ownEntry) */
#define DHT_PEER_ADDR_KNOWN     4
#define DHT_PEER_PUBLISHED      5

/* Interface with Real DHT Implementation */
#define DHT_MODE_SEARCH         1
#define DHT_MODE_PUBLISH        1
#define DHT_MODE_NOTIFY         2
#define DHT_MODE_BOOTSTRAP	3


/* TIMEOUTS: Reference Values are set here... */

#define DHT_SEARCH_PERIOD       1800 /* PeerKeys: if we haven't found them: 30 min */
#define DHT_CHECK_PERIOD        1800 /* PeerKeys: re-lookup peer: 30 min */
#define DHT_PUBLISH_PERIOD      1800 /* OwnKey: 30 min */
#define DHT_NOTIFY_PERIOD       300  /* 5 min - Notify Check period */

/* TTLs for DHTs posts */
#define DHT_TTL_PUBLISH         (DHT_PUBLISH_PERIOD + 120)  // for a little overlap.
#define DHT_TTL_NOTIFY          (DHT_NOTIFY_PERIOD  + 60)   // for time to find it...
#define DHT_TTL_BOOTSTRAP       (DHT_PUBLISH_PERIOD)        // To start with.

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

class p3DhtMgr: public pqiNetAssistConnect, public RsThread
{
    /*
     */
public:
    p3DhtMgr(std::string id, pqiConnectCb *cb);

    /********** External DHT Interface ************************
     * These Functions are the external interface
     * for the DHT, and must be non-blocking and return quickly
     */

    /* OVERLOADED From pqiNetAssistConnect. */

    virtual void enable(bool on);
    virtual void shutdown();
    virtual void restart();

    virtual bool getEnabled(); /* on */
    virtual bool getActive();  /* actually working */

    virtual void	setBootstrapAllowed(bool on);
    virtual bool 	getBootstrapAllowed();

    /* set key data */
    virtual bool 	setExternalInterface(struct sockaddr_in laddr,
                                       struct sockaddr_in raddr, uint32_t type);

    /* add / remove peers */
    virtual bool 	findPeer(std::string id);
    virtual bool 	dropPeer(std::string id);

    /* post DHT key saying we should connect (callback when done) */
    virtual bool 	notifyPeer(std::string id);

    /* extract current peer status */
    virtual bool 	getPeerStatus(std::string id,
                                struct sockaddr_in &laddr, struct sockaddr_in &raddr,
                                uint32_t &type, uint32_t &mode);

    /* stun */
    virtual bool 	enableStun(bool on);
    virtual bool 	addStun(std::string id);
    //doneStun();

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

    virtual bool dhtResultBootstrap(std::string idhash);

protected:

    /* can block briefly (called only from thread) */
    virtual bool dhtPublish(std::string id,
                            struct sockaddr_in &laddr,
                            struct sockaddr_in &raddr,
                            uint32_t type, std::string sign);

    virtual bool dhtNotify(std::string peerid, std::string ownId,
                           std::string sign);

    virtual	bool dhtSearch(std::string id, uint32_t mode);

    virtual bool dhtBootstrap(std::string storehash, std::string ownIdHash,
                              std::string sign); /* to publish bootstrap */



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

    virtual bool    dhtInit();
    virtual bool	dhtShutdown();
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
    int 	checkStunState_Active(); /* when in active state */
    int 	doStun();
    int 	checkPeerDHTKeys();
    int 	checkOwnDHTKeys();
    int 	checkNotifyDHT();

    void 	clearDhtData();

    /* IP Bootstrap */
    bool 	getDhtBootstrapList();
    std::string BootstrapId(uint32_t bin);
    std::string randomBootstrapId();

    /* other feedback through callback */
    // use pqiNetAssistConnect.. version pqiConnectCb *connCb;

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

    bool   mBootstrapAllowed;
    time_t mLastBootstrapListTS;
};


#endif // P3DHDHTMGR_H


