#ifndef _RS_UPNP_IFACE_H
#define _RS_UPNP_IFACE_H

#include <string.h>

#include "util/rsthreads.h"
#include <string>
#include <map>

/* platform independent networking... */
#include "pqi/pqinetwork.h"
#include "pqi/pqiaddrstore.h"

class upnpentry
{
	public:
	std::string name;
	std::string id;
	struct sockaddr_in addr;
	unsigned int flags;
	int status;
	int lastTs;
};

class upnpforward
{
	public:
	std::string name;
	unsigned int flags;
	struct sockaddr_in iaddr;
	struct sockaddr_in eaddr;
	int status;
	int lastTs;
};

#define RS_UPNP_S_UNINITIALISED  0
#define RS_UPNP_S_UNAVAILABLE    1
#define RS_UPNP_S_READY          2
#define RS_UPNP_S_FAILED         3
#define RS_UPNP_S_ACTIVE         4

class uPnPConfigData;

class upnphandler: public RsThread
{
	public:

	upnphandler()
	:toShutdown(false), toEnable(false), 
	toStart(false), toStop(false),
	eport(0), 
	upnpState(RS_UPNP_S_UNINITIALISED)
	{
		return;
	}

	~upnphandler()
	{
		return;
	}

	/* RsIface */
void    enableUPnP(bool active)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	toEnable = active;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}

	/* RsIface */
void    shutdownUPnP()
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	toShutdown = true;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}

void    setupUPnPForwarding()
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	toStart = true;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}

void    shutdownUPnPForwarding()
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	toStop = true;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}


	/* the address that the listening port is on */
void    setInternalAddress(struct sockaddr_in iaddr_in)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	iaddr = iaddr_in;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}

void    setExternalPort(unsigned short eport)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	/* flag both shutdown/start -> for restart */
	toStop  = true;
	toStart = true;

	/* TODO */

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/
}

	/* as determined by uPnP */
bool    getInternalAddress(struct sockaddr_in &addr)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	addr = upnp_iaddr;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/

	return (upnpState >= RS_UPNP_S_READY);
}

bool    getExternalAddress(struct sockaddr_in &addr)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	addr = upnp_eaddr;

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/

	return (upnpState >= RS_UPNP_S_READY);
}

int    getUPnPStatus(upnpentry &ent)
{
	dataMtx.lock();   /***  LOCK MUTEX  ***/

	/* TODO - define data structure first */

	dataMtx.unlock(); /*** UNLOCK MUTEX ***/

	return upnpState;
}


int 	init();
int 	shutdown();
int	print();

	/* must run thread */
virtual void run();

	private:

bool initUPnPState();
void checkUPnPState();
bool printUPnPState();
bool updateUPnP();


	/* Mutex for data below */
	RsMutex dataMtx;

 	/* requested from rs */
	bool toShutdown; /* if set shuts down the thread. */

	bool toEnable;   /* overall on/off switch */
	bool toStart;  /* if set start forwarding */
	bool toStop;   /* if set stop  forwarding */

	struct sockaddr_in iaddr;
	unsigned short eport;

	/* info from upnp */
	unsigned int upnpState;
	uPnPConfigData *upnpConfig;

	struct sockaddr_in upnp_iaddr;
	struct sockaddr_in upnp_eaddr;

	/* active port forwarding */
	std::list<upnpforward> activeForwards;

};

#endif /* _RS_UPNP_IFACE_H */
