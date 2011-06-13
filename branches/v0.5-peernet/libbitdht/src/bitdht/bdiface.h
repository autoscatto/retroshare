#ifndef BIT_DHT_INTERFACE_H
#define BIT_DHT_INTERFACE_H

/*
 * bitdht/bdiface.h
 *
 * BitDHT: An Flexible DHT library.
 *
 * Copyright 2010 by Robert Fernie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 3 as published by the Free Software Foundation.
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
 * Please report all bugs and problems to "bitdht@lunamutt.com".
 *
 */

#include <iosfwd>
#include <map>
#include <string>
#include <list>
#include <inttypes.h>

#include "util/bdnet.h"

/*
 * Functions and Classes required for Interfacing with the BitDht.
 * This should be the sole header file required to talk to Dht.
 * ... though setting it up will require including udpbitdht.h as well.
 * 
 */

#define BITDHT_KEY_LEN 20
#define BITDHT_KEY_INTLEN 5
#define BITDHT_KEY_BITLEN 160




#define BITDHT_MAX_PKTSIZE 1024

#define BITDHT_TTL 64

#define BITDHT_SEARCH_ONE_SHOT 		1
#define BITDHT_SEARCH_REPEATING		2

class bdNodeId
{
        public:
        unsigned char data[BITDHT_KEY_LEN];
};

class bdMetric: public bdNodeId {};

class bdId
{
        public:

        bdId();
        bdId(bdNodeId in_id, struct sockaddr_in in_addr);

        struct sockaddr_in addr;
        bdNodeId id;
};

#define BITDHT_LIKELY_SAME_NO			0x00000000
#define BITDHT_LIKELY_SAME_YES			0x00000001
#define BITDHT_LIKELY_SAME_PORT_CHANGED		0x00000002
#define BITDHT_LIKELY_SAME_LOC_CHANGED		0x00000004
#define BITDHT_LIKELY_SAME_IDENTICAL		0x00000008


class bdDhtFunctions
{
	public:

//	bdDhtFunctions();
	/* setup variables */
virtual uint16_t bdNumBuckets() = 0;
virtual uint16_t bdNodesPerBucket() = 0; /* used for query + bdspace */
virtual uint16_t bdBucketBitSize() = 0;

virtual int bdDistance(const bdNodeId *n1, const bdNodeId *n2, bdMetric *metric) = 0;
virtual int bdBucketDistance(const bdNodeId *n1, const bdNodeId *n2) = 0;
virtual int bdBucketDistance(const bdMetric *metric) = 0;

virtual uint32_t bdLikelySameNode(const bdId *id1, const bdId *id2) = 0;

virtual void bdRandomMidId(const bdNodeId *target, const bdNodeId *other, bdNodeId *mid) = 0;

virtual void bdPrintId(std::ostream &out, const bdId *a) = 0;
virtual void bdPrintNodeId(std::ostream &out, const bdNodeId *a) = 0;

};





/* peer flags
 * order is important!
 * higher bits = more priority.
 * BITDHT_PEER_STATUS_RECVPONG
 * BITDHT_PEER_STATUS_RECVNODES
 * BITDHT_PEER_STATUS_RECVHASHES
 * BITDHT_PEER_STATUS_DHT_ENGINE  (dbXXxx)
 * BITDHT_PEER_STATUS_DHT_APPL    (XXRSxx)
 * BITDHT_PEER_STATUS_DHT_VERSION (XXxx50)
 * 
 */

#define 	BITDHT_PEER_STATUS_MASK_RECVD		0x000000ff
#define 	BITDHT_PEER_STATUS_MASK_DHT		0x0000ff00
#define 	BITDHT_PEER_STATUS_MASK_KNOWN		0x00ff0000

#define 	BITDHT_PEER_STATUS_RECV_PONG		0x00000001
#define 	BITDHT_PEER_STATUS_RECV_NODES		0x00000002
#define 	BITDHT_PEER_STATUS_RECV_HASHES		0x00000004
#define 	BITDHT_PEER_STATUS_RECV_CONNECT_MSG	0x00000008

#define 	BITDHT_PEER_STATUS_DHT_ENGINE		0x00000100
#define 	BITDHT_PEER_STATUS_DHT_APPL		0x00000200
#define 	BITDHT_PEER_STATUS_DHT_VERSION		0x00000400

#define 	BITDHT_PEER_STATUS_DHT_WHITELIST	0x00010000
#define 	BITDHT_PEER_STATUS_DHT_FOF		0x00020000
#define 	BITDHT_PEER_STATUS_DHT_FRIEND		0x00040000



#define 	BITDHT_CONNECT_MASK_MODE		0x000000ff
#define 	BITDHT_CONNECT_MASK_ANSWER		0x0000ff00

#define 	BITDHT_CONNECT_MODE_DIRECT		0x00000001
#define 	BITDHT_CONNECT_MODE_PROXY		0x00000002
#define 	BITDHT_CONNECT_MODE_RELAY		0x00000004

#define 	BITDHT_CONNECT_ANSWER_OKAY		0x00000100
#define 	BITDHT_CONNECT_ANSWER_NOK		0x00000200
#define 	BITDHT_CONNECT_ANSWER_NCONN		0x00000400

#define 	BITDHT_CONNECT_ANSWER_TOAUTH		0x00001000


/* Definitions of bdSpace Peer and Bucket are publically available, 
 * so we can expose the bucket entries for the gui.
 */

class bdPeer
{
	public:
	bdId   mPeerId;
	uint32_t mPeerFlags;
	time_t mLastSendTime;
	time_t mLastRecvTime;
	time_t mFoundTime;     /* time stamp that peer was found */
};
	
class bdBucket
{
	public:
	
	bdBucket();
	/* list so we can queue properly */
	std::list<bdPeer> entries;
};


/* Status options */
#define BITDHT_QUERY_READY		1
#define BITDHT_QUERY_QUERYING           2
#define BITDHT_QUERY_FAILURE            3
#define BITDHT_QUERY_FOUND_CLOSEST      4
#define BITDHT_QUERY_PEER_UNREACHABLE   5
#define BITDHT_QUERY_SUCCESS            6

/* Query Flags */
#define BITDHT_QFLAGS_NONE		0
#define BITDHT_QFLAGS_DISGUISE		1
#define BITDHT_QFLAGS_DO_IDLE		2
#define BITDHT_QFLAGS_INTERNAL		4  // means it runs through startup.

/* Connect Callback Flags */
#define BITDHT_CONNECT_CB_AUTH		1
#define BITDHT_CONNECT_CB_PENDING	2
#define BITDHT_CONNECT_CB_START		3
#define BITDHT_CONNECT_CB_PROXY		4
#define BITDHT_CONNECT_CB_FAILED	5

#define BD_PROXY_CONNECTION_UNKNOWN_POINT       0
#define BD_PROXY_CONNECTION_START_POINT         1
#define BD_PROXY_CONNECTION_MID_POINT           2
#define BD_PROXY_CONNECTION_END_POINT           3

class BitDhtCallback
{
	public:
//        ~BitDhtCallback();

		// dummy cos not needed for standard dht behaviour;
virtual int dhtNodeCallback(const bdId *  /*id*/, uint32_t /*peerflags*/)  { return 0; } 

		// must be implemented.
virtual int dhtPeerCallback(const bdId *id, uint32_t status) = 0;
virtual int dhtValueCallback(const bdNodeId *id, std::string key, uint32_t status) = 0;

		// connection callback. Not required for basic behaviour, but forced for initial development.
virtual int dhtConnectCallback(const bdId *srcId, const bdId *proxyId, const bdId *destId, 
					uint32_t mode, uint32_t point, uint32_t cbtype) = 0; /*  { return 0; }  */

};


class BitDhtInterface
{
	public:

        /***** Request Lookup (DHT Peer & Keyword) *****/
virtual void addFindNode(bdNodeId *id, uint32_t mode) = 0;
virtual void removeFindNode(bdNodeId *id) = 0;
virtual void findDhtValue(bdNodeId *id, std::string key, uint32_t mode) = 0;

	/***** Connections Requests *****/
virtual void ConnectionRequest(struct sockaddr_in *laddr, bdNodeId *target, uint32_t mode) = 0;
virtual void ConnectionAuth(bdId *srcId, bdId *proxyId, bdId *destId, uint32_t mode, uint32_t loc, uint32_t answer) = 0;

        /***** Add / Remove Callback Clients *****/
virtual void addCallback(BitDhtCallback *cb) = 0;
virtual void removeCallback(BitDhtCallback *cb) = 0;

	/***** Get Results Details *****/
virtual int getDhtPeerAddress(const bdNodeId *id, struct sockaddr_in &from) = 0;
virtual int getDhtValue(const bdNodeId *id, std::string key, std::string &value) = 0;
virtual int getDhtBucket(const int idx, bdBucket &bucket) = 0;

        /* stats and Dht state */
virtual int startDht() = 0;
virtual int stopDht() = 0;
virtual int stateDht() = 0; /* STOPPED, STARTING, ACTIVE, FAILED */
virtual uint32_t statsNetworkSize() = 0;
virtual uint32_t statsBDVersionSize() = 0; /* same version as us! */
};

#endif
