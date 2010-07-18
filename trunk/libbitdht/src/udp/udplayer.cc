/*
 * udp/udplayer.cc
 *
 * BitDHT: An Flexible DHT library.
 *
 * Copyright 2004-2010 by Robert Fernie
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

#include "udp/udplayer.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <stdlib.h>

/***
 * #define UDP_ENABLE_BROADCAST		1
 * #define UDP_LOOPBACK_TESTING		1
 * #define DEBUG_UDP_LAYER 		1
 ***/

static const int UDP_DEF_TTL = 64;

/* NB: This #define makes the listener open 0.0.0.0:X port instead
 * of a specific port - this helps library communicate on systems
 * with multiple interfaces or unique network setups.
 *
 * - It should always be used!
 *
 * #define OPEN_UNIVERSAL_PORT 1
 *
 */

#define OPEN_UNIVERSAL_PORT 1


class   udpPacket
{
	public:
	udpPacket(struct sockaddr_in *addr, void *dta, int dlen)
	:raddr(*addr), len(dlen)
	{
		data = malloc(len);
		memcpy(data, dta, len);
	}

	~udpPacket()
	{
		if (data)
		{
			free(data);
			data = NULL;
			len = 0;
		}
	}

	struct sockaddr_in raddr;
	void *data;
	int len;
};

std::ostream &operator<<(std::ostream &out, const struct sockaddr_in &addr)
{
	out << "[" << inet_ntoa(addr.sin_addr) << ":";
	out << htons(addr.sin_port) << "]";
	return out;
}

bool operator==(const struct sockaddr_in &addr, const struct sockaddr_in &addr2)
{
	if (addr.sin_family != addr2.sin_family)
		return false;
	if (addr.sin_addr.s_addr != addr2.sin_addr.s_addr)
		return false;
	if (addr.sin_port != addr2.sin_port)
		return false;
	return true;
}


bool operator<(const struct sockaddr_in &addr, const struct sockaddr_in &addr2)
{
	if (addr.sin_family != addr2.sin_family)
		return (addr.sin_family < addr2.sin_family);
	if (addr.sin_addr.s_addr != addr2.sin_addr.s_addr)
		return (addr.sin_addr.s_addr < addr2.sin_addr.s_addr);
	if (addr.sin_port != addr2.sin_port)
		return (addr.sin_port < addr2.sin_port);
	return false;
}

std::string printPkt(void *d, int size)
{
	std::ostringstream out;
	out << "Packet:" << "**********************";
	for(int i = 0; i < size; i++)
	{
		if (i % 16 == 0)
			out << std::endl;
		out << std::hex << std::setw(2) << (unsigned int) ((unsigned char *) d)[i] << " ";
	}
	out << std::endl << "**********************";
	out << std::endl;
	return out.str();
}


std::string printPktOffset(unsigned int offset, void *d, unsigned int size)
{
	std::ostringstream out;
	out << "Packet:" << "**********************";
	out << std::endl;
	out << "Offset: " << std::hex << offset << " -> " << offset + size;
	out << std::endl;
	out << "Packet:" << "**********************";

	unsigned int j = offset % 16;
	if (j != 0)
	{
	  out << std::endl;
	  out << std::hex << std::setw(6) << (unsigned int) offset - j;
	  out << ": ";
	  for(unsigned int i = 0; i < j; i++)
	  {
		out << "xx ";
	  }
	}
	for(unsigned int i = offset; i < offset + size; i++)
	{
		if (i % 16 == 0)
		{
			out << std::endl;
			out << std::hex << std::setw(6) << (unsigned int) i;
			out << ": ";
		}
		out << std::hex << std::setw(2) << (unsigned int) ((unsigned char *) d)[i-offset] << " ";
	}
	out << std::endl << "**********************";
	out << std::endl;
	return out.str();
}



UdpLayer::UdpLayer(UdpReceiver *udpr, struct sockaddr_in &local)
	:recv(udpr), laddr(local), errorState(0), ttl(UDP_DEF_TTL)
{
	openSocket();
	return;
}

int     UdpLayer::status(std::ostream &out)
{
	out << "UdpLayer::status()" << std::endl;
	out << "localaddr: " << laddr << std::endl;
	out << "sockfd: " << sockfd << std::endl;
	out << std::endl;
	return 1;
}

int UdpLayer::reset(struct sockaddr_in &local)
{
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::reset()" << std::endl;
#endif

	/* stop the old thread */
	{
		bdStackMutex stack(sockMtx);   /********** LOCK MUTEX *********/
#ifdef DEBUG_UDP_LAYER
		std::cerr << "UdpLayer::reset() setting stopThread flag" << std::endl;
#endif
		stopThread = true;
	}
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::reset() joining" << std::endl;
#endif

	join(); 

#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::reset() closing socket" << std::endl;
#endif
	closeSocket();

#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::reset() resetting variables" << std::endl;
#endif
	laddr = local;
	errorState = 0;
	ttl = UDP_DEF_TTL;

#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::reset() opening socket" << std::endl;
#endif
	openSocket();

	return 1 ;
}


int UdpLayer::closeSocket()
{
	/* close socket if open */
	sockMtx.lock();   /********** LOCK MUTEX *********/

	if (sockfd > 0)
	{
       		bdnet_close(sockfd);
	}

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/
	return 1;
}

void UdpLayer::run()
{
	return recv_loop();
}

/* higher level interface */
void UdpLayer::recv_loop()
{
	int maxsize = 16000;
	void *inbuf = malloc(maxsize);

        int status;
        struct timeval timeout;

	while(1)
	{
                fd_set rset;
                for(;;) 
		{
			/* check if we need to stop */
			bool toStop = false;
			{
				bdStackMutex stack(sockMtx);   /********** LOCK MUTEX *********/
				toStop = stopThread;
			}
			
			if (toStop)
			{
#ifdef DEBUG_UDP_LAYER
				std::cerr << "UdpLayer::recv_loop() stopping thread" << std::endl;
#endif
				stop();
			}

                        FD_ZERO(&rset);
                        FD_SET(sockfd, &rset);
                        timeout.tv_sec = 0;
                        timeout.tv_usec = 500000;       /* 500 ms timeout */
                        status = select(sockfd+1, &rset, NULL, NULL, &timeout);
                        if (status > 0)
			{
                                break;  /* data available, go read it */
                        } 
			else if (status < 0) 
			{
#ifdef DEBUG_UDP_LAYER
                                std::cerr << "UdpLayer::recv_loop() Error: " << bdnet_errno() << std::endl;
#endif
                        }
                };      

		int nsize = maxsize;
		struct sockaddr_in from;
		if (0 < receiveUdpPacket(inbuf, &nsize, from))
		{
#ifdef DEBUG_UDP_LAYER
			std::cerr << "UdpLayer::readPkt()  from : " << from << std::endl;
			std::cerr << printPkt(inbuf, nsize);
#endif
			// send to reciever.
			recv -> recvPkt(inbuf, nsize, from);
		}
		else
		{
#ifdef DEBUG_UDP_LAYER
			std::cerr << "UdpLayer::readPkt() not ready" << from;
			std::cerr << std::endl;
#endif
		}
	}
	return;
}


int UdpLayer::sendPkt(const void *data, int size, sockaddr_in &to, int ttl)
{
	/* if ttl is different -> set it */
	if (ttl != getTTL())
	{
		setTTL(ttl);
	}

	/* and send! */
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::sendPkt()  to: " << to << std::endl;
	std::cerr << printPkt((void *) data, size);
#endif
	sendUdpPacket(data, size, to);
	return size;
}

/* setup connections */
int UdpLayer::openSocket()	
{
	sockMtx.lock();   /********** LOCK MUTEX *********/

	/* make a socket */
       	sockfd = bdnet_socket(PF_INET, SOCK_DGRAM, 0);
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UpdStreamer::openSocket()" << std::endl;
#endif
	/* bind to address */


#ifdef UDP_LOOPBACK_TESTING
        inet_aton("127.0.0.1", &(laddr.sin_addr));
#endif

#ifdef OPEN_UNIVERSAL_PORT
        struct sockaddr_in tmpaddr = laddr;
        tmpaddr.sin_addr.s_addr = 0;
	if (0 != bdnet_bind(sockfd, (struct sockaddr *) (&tmpaddr), sizeof(tmpaddr)))
#else
	if (0 != bdnet_bind(sockfd, (struct sockaddr *) (&laddr), sizeof(laddr)))
#endif
	{
#ifdef DEBUG_UDP_LAYER
		std::cerr << "Socket Failed to Bind to : " << laddr << std::endl;
		std::cerr << "Error: " << bdnet_errno() << std::endl;
#endif
		errorState = EADDRINUSE;
		//exit(1);

		sockMtx.unlock(); /******** UNLOCK MUTEX *********/
		return -1;
	}

	if (-1 == bdnet_fcntl(sockfd, F_SETFL, O_NONBLOCK))
	{
#ifdef DEBUG_UDP_LAYER
		std::cerr << "Failed to Make Non-Blocking" << std::endl;
#endif
	}

#ifdef UDP_ENABLE_BROADCAST
	/* Setup socket for broadcast. */
	int val = 1;
	if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &val, sizeof(int)))
	{
#ifdef DEBUG_UDP_LAYER
		std::cerr << "Failed to Make Socket Broadcast" << std::endl;
#endif
	}
#endif

	errorState = 0;

#ifdef DEBUG_UDP_LAYER
	std::cerr << "Socket Bound to : " << laddr << std::endl;
#endif

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/

#ifdef DEBUG_UDP_LAYER
	std::cerr << "Setting TTL to " << UDP_DEF_TTL << std::endl;
#endif
	setTTL(UDP_DEF_TTL);

	// start up our thread.
	{
		bdStackMutex stack(sockMtx);   /********** LOCK MUTEX *********/
		stopThread = false;
	}
	start();

	return 1;

}

int UdpLayer::setTTL(int t)
{
	sockMtx.lock();   /********** LOCK MUTEX *********/

	int err = bdnet_setsockopt(sockfd, IPPROTO_IP, IP_TTL, &t, sizeof(int));
	ttl = t;

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/

#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::setTTL(" << t << ") returned: " << err;
	std::cerr << std::endl;
#endif

	return err;
}

int UdpLayer::getTTL()
{
	sockMtx.lock();   /********** LOCK MUTEX *********/

	int t = ttl;

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/

	return t;
}

/* monitoring / updates */
int UdpLayer::okay()
{
	sockMtx.lock();   /********** LOCK MUTEX *********/

	bool nonFatalError = ((errorState == 0) ||
				(errorState == EAGAIN) ||
				(errorState == EINPROGRESS));

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/

#ifdef DEBUG_UDP_LAYER
	if (!nonFatalError)
	{
		std::cerr << "UdpLayer::NOT okay(): Error: " << errorState << std::endl;
	}

#endif

	return nonFatalError;
}

int UdpLayer::tick()
{
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::tick()" << std::endl;
#endif
	return 1;
}

/******************* Internals *************************************/

int UdpLayer::receiveUdpPacket(void *data, int *size, struct sockaddr_in &from)
{
	struct sockaddr_in fromaddr;
	socklen_t fromsize = sizeof(fromaddr);
	int insize = *size;

	sockMtx.lock();   /********** LOCK MUTEX *********/

	insize = bdnet_recvfrom(sockfd,data,insize,0,
			(struct sockaddr*)&fromaddr,&fromsize);

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/

	if (0 < insize)
	{
#ifdef DEBUG_UDP_LAYER
		std::cerr << "receiveUdpPacket() from: " << fromaddr;
		std::cerr << " Size: " << insize;
		std::cerr << std::endl;
#endif
		*size = insize;
		from = fromaddr;
		return insize;
	}
	return -1;
}

int UdpLayer::sendUdpPacket(const void *data, int size, struct sockaddr_in &to)
{
	/* send out */
#ifdef DEBUG_UDP_LAYER
	std::cerr << "UdpLayer::sendUdpPacket(): size: " << size;
	std::cerr << " To: " << to << std::endl;
#endif
	struct sockaddr_in toaddr = to;

	sockMtx.lock();   /********** LOCK MUTEX *********/

	bdnet_sendto(sockfd, data, size, 0, 
			   (struct sockaddr *) &(toaddr), 
				sizeof(toaddr));

	sockMtx.unlock(); /******** UNLOCK MUTEX *********/
	return 1;
}


/**************************** LossyUdpLayer - for Testing **************/


LossyUdpLayer::LossyUdpLayer(UdpReceiver *udpr, 
			struct sockaddr_in &local, double frac)
	:UdpLayer(udpr, local), lossFraction(frac)
{
	return;
}
LossyUdpLayer::~LossyUdpLayer() { return; }

int LossyUdpLayer::receiveUdpPacket(void *data, int *size, struct sockaddr_in &from)
{
	double prob = (1.0 * (rand() / (RAND_MAX + 1.0)));
	
	if (prob < lossFraction)
	{
	/* but discard */
		if (0 < UdpLayer::receiveUdpPacket(data, size, from))
		{
			std::cerr << "LossyUdpLayer::receiveUdpPacket() Dropping packet!";
			std::cerr << std::endl;
			std::cerr << printPkt(data, *size);
			std::cerr << std::endl;
			std::cerr << "LossyUdpLayer::receiveUdpPacket() Packet Dropped!";
			std::cerr << std::endl;
		}
	
		size = 0;
		return -1;
	
	}
	
	// otherwise read normally;
	return UdpLayer::receiveUdpPacket(data, size, from);
}
	
int LossyUdpLayer::sendUdpPacket(const void *data, int size, struct sockaddr_in &to)
{
	double prob = (1.0 * (rand() / (RAND_MAX + 1.0)));
	
	if (prob < lossFraction)
	{
		/* discard */
	
		std::cerr << "LossyUdpLayer::sendUdpPacket() Dropping packet!";
		std::cerr << std::endl;
		std::cerr << printPkt((void *) data, size);
		std::cerr << std::endl;
		std::cerr << "LossyUdpLayer::sendUdpPacket() Packet Dropped!";
		std::cerr << std::endl;
	
		return size;
	}
	
	// otherwise read normally;
	return UdpLayer::sendUdpPacket(data, size, to);
}
	
