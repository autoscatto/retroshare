#ifndef CONNECT_STATUS_BOX_H
#define CONNECT_STATUS_BOX_H

/* a connect state box */

#define CSB_START		1
#define CSB_DIRECT_ATTEMPT	2
#define CSB_PROXY_ATTEMPT	3
#define CSB_RELAY_ATTEMPT	4
#define CSB_REVERSE_WAIT	5
#define CSB_FAILED_WAIT		6
#define CSB_CONNECTED		7


#define CSB_NETSTATE_UNKNOWN		0
#define CSB_NETSTATE_FORWARD		1
#define CSB_NETSTATE_STABLENAT		2
#define CSB_NETSTATE_FIREWALLED		3

#define CSB_CONNECT_DIRECT		1
#define CSB_CONNECT_UNREACHABLE		2

/* return values */
#define CSB_ACTION_MASK_MODE		0x00ff
#define CSB_ACTION_MASK_PORT		0xff00

#define CSB_ACTION_WAIT			0x0001
#define CSB_ACTION_DIRECT_CONN		0x0002
#define CSB_ACTION_PROXY_CONN		0x0004
#define CSB_ACTION_RELAY_CONN		0x0008

#define CSB_ACTION_DHT_PORT		0x0100
#define CSB_ACTION_PROXY_PORT		0x0200

/* update input */
#define	CSB_UPDATE_NONE			0x0000
#define	CSB_UPDATE_CONNECTED		0x0001
#define	CSB_UPDATE_DISCONNECTED		0x0002
#define	CSB_UPDATE_AUTH_DENIED		0x0003
#define	CSB_UPDATE_FAILED_ATTEMPT	0x0004
#define	CSB_UPDATE_MODE_UNAVAILABLE	0x0005

#include <iosfwd>
#include <string>

#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

class PeerConnectStateBox
{
	public:
	PeerConnectStateBox();

	uint32_t connectCb(uint32_t cbtype, uint32_t netmode, uint32_t nattype);
	uint32_t updateCb(uint32_t updateType);

	bool shouldUseProxyPort(uint32_t netmode, uint32_t nattype);

	std::string connectState();

	std::string mPeerId;
	
	private:

	uint32_t connectCb_direct();
	uint32_t connectCb_unreachable();

	void errorMsg(std::ostream &out, std::string msg, uint32_t updateParam);
	void stateMsg(std::ostream &out, std::string msg, uint32_t updateParam);


	uint32_t mState;
	uint32_t mNetState;
	time_t mAttemptTS;
	uint32_t mNoAttempts;
};


#endif