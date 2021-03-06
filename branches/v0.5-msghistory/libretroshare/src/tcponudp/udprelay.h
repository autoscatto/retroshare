#ifndef RS_UDP_RELAY_H
#define RS_UDP_RELAY_H

/*
 * tcponudp/udprelay.h
 *
 * libretroshare.
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
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */

#include "tcponudp/udppeer.h"
#include <vector>

class UdpRelayAddrSet;

class UdpRelayAddrSet
{
	public:
	UdpRelayAddrSet();
	UdpRelayAddrSet(const sockaddr_in *ownAddr, const sockaddr_in *destAddr);

	UdpRelayAddrSet flippedSet();

	struct sockaddr_in mSrcAddr; /* msg source */
	struct sockaddr_in mDestAddr; /* final destination */
};

int operator<(const UdpRelayAddrSet &a, const UdpRelayAddrSet &b);
	
class UdpRelayProxy
{
	public:
	UdpRelayProxy();
	UdpRelayProxy(UdpRelayAddrSet *addrSet, int relayClass);

	UdpRelayAddrSet mAddrs;
	double mBandwidth;
	uint32_t mDataSize;
	time_t mLastBandwidthTS;
	time_t mLastTS;

	int mRelayClass;
};


class UdpRelayEnd
{
	public:

	UdpRelayEnd();
	UdpRelayEnd(UdpRelayAddrSet *endPoints, const struct sockaddr_in *proxyaddr);

	struct sockaddr_in mLocalAddr; 
	struct sockaddr_in mProxyAddr; 
	struct sockaddr_in mRemoteAddr; 
};

std::ostream &operator<<(std::ostream &out, const UdpRelayAddrSet &uras);
std::ostream &operator<<(std::ostream &out, const UdpRelayProxy &urp);
std::ostream &operator<<(std::ostream &out, const UdpRelayEnd &ure);

/* we define a couple of classes (determining which class is done elsewhere)
 * There will be various maximums for each type.
 * Ideally you want to allow your friends to use your proxy in preference
 * to randoms.
 *
 * At N x 2 x maxBandwidth.
 *
 * 10 x 2 x 1Kb/s => 20Kb/s In and Out. (quite a bit!)
 * 20 x 2 x 1Kb/s => 40Kb/s Huge.
 */

#define UDP_RELAY_DEFAULT_COUNT_ALL	10 
#define UDP_RELAY_FRAC_GENERAL		(0.2)
#define UDP_RELAY_FRAC_FOF		(0.5)
#define UDP_RELAY_FRAC_FRIENDS		(0.8)

#define UDP_RELAY_NUM_CLASS		4

#define UDP_RELAY_CLASS_ALL		0
#define UDP_RELAY_CLASS_GENERAL		1
#define UDP_RELAY_CLASS_FOF		2
#define UDP_RELAY_CLASS_FRIENDS		3

#define STD_RELAY_TTL	64

class UdpRelayReceiver: public UdpSubReceiver
{
	public:

	UdpRelayReceiver(UdpPublisher *pub);
virtual ~UdpRelayReceiver();

	/* add a TCPonUDP stream (ENDs) */
int	addUdpPeer(UdpPeer *peer, UdpRelayAddrSet *endPoints, const struct sockaddr_in *proxyaddr);
int 	removeUdpPeer(UdpPeer *peer);

	/* add a Relay Point (for the Relay).
	 * These don't have to be explicitly removed.
	 * They will be timed out when 
	 * the end-points drop the connections 
	 */

	int addUdpRelay(UdpRelayAddrSet *addrs, int classIdx);
	int removeUdpRelay(UdpRelayAddrSet *addrs);

	/* Need some stats, to work out how many relays we are supporting */
	int checkRelays();

	int setRelayTotal(int count); /* sets all the Relay Counts (frac based on total) */
	int setRelayClassMax(int classIdx, int count); /* set a specific class maximum */
	int getRelayClassMax(int classIdx);
	int getRelayCount(int classIdx); /* how many relays (of this type) do we have */
	int RelayStatus(std::ostream &out);

	/* Extract Relay Data */
	int getRelayEnds(std::list<UdpRelayEnd> &relayEnds); 		
	int getRelayProxies(std::list<UdpRelayProxy> &relayProxies); 
	
	/* callback for recved data (overloaded from UdpReceiver) */
virtual int recvPkt(void *data, int size, struct sockaddr_in &from);

	/* wrapper function for relay (overloaded from UdpSubReceiver) */
virtual int sendPkt(const void *data, int size, const struct sockaddr_in &to, int ttl);

int     status(std::ostream &out);

	private:

	int removeUdpRelay_relayLocked(UdpRelayAddrSet *addrs);
	int installRelayClass_relayLocked(int classIdx);
	int removeRelayClass_relayLocked(int classIdx);

	/* Unfortunately, Due the reentrant nature of this classes activities...
	 * the SendPkt() must be callable from inside RecvPkt().
	 * This means we need two seperate mutexes.
	 *  - one for UdpPeer's, and one for Relay Data.
	 *
	 * care must be taken to lock these mutex's in a consistent manner to avoid deadlock.
	 *  - You are not allowed to hold both at the same time!
	 */

	RsMutex udppeerMtx; /* for all class data (below) */
	
	std::map<struct sockaddr_in, UdpPeer *> mPeers; /* indexed by <dest> */

	RsMutex relayMtx; /* for all class data (below) */

	std::vector<int> mClassLimit, mClassCount;
	std::map<struct sockaddr_in, UdpRelayEnd> mStreams; /* indexed by <dest> */
	std::map<UdpRelayAddrSet, UdpRelayProxy> mRelays; /* indexed by <src,dest> */

	void *mTmpSendPkt;
	uint32_t mTmpSendSize;

};

/* utility functions for creating / extracting UdpRelayPackets */
int isUdpRelayPacket(const void *data, const int size);
int getPacketFromUdpRelayPacket(const void *data, const int size, void **realdata, int *realsize);

int createRelayUdpPacket(const void *data, const int size, void *newpkt, int newsize, UdpRelayEnd *ure);
int extractUdpRelayAddrSet(const void *data, const int size, UdpRelayAddrSet &addrSet);




#endif
