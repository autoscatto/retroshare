/*
 * "$Id: tcpstream.cc,v 1.11 2007-03-01 01:09:39 rmf24 Exp $"
 *
 * TCP-on-UDP (tou) network interface for RetroShare.
 *
 * Copyright 2004-2006 by Robert Fernie.
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



 
#include "tcpstream.h"
#include <iostream>
#include <assert.h>
#include <errno.h>
#include <math.h>

#include <sys/time.h>
#include <time.h>


/*
 * #define DEBUG_TCP_STREAM 1
 */

/*
 *#define DEBUG_TCP_STREAM_EXTRA 1
 */

/*
 * #define TCP_NO_PARTIAL_READ 1
 */

#ifdef DEBUG_TCP_STREAM
int	checkData(uint8 *data, int size, int idx);
int	setupBinaryCheck(std::string fname);
#endif

static const uint32 kMaxQueueSize = 100;
static const uint32 kMaxPktRetransmit = 20;
static const uint32 kMaxSynPktRetransmit = 200; // max TTL of 40?
static const int    TCP_STD_TTL = 64;
static const int    TCP_STARTUP_COUNT_PER_TTL = 2;

static const double RTT_ALPHA = 0.875;

// platform independent fractional timestamp.
static double getCurrentTS();


TcpStream::TcpStream(UdpLayer *lyr)
	:inSize(0), outSizeRead(0), outSizeNet(0), 
	state(TCP_CLOSED), 
        inStreamActive(false),
        outStreamActive(false),
	outSeqno(0), outAcked(0), outWinSize(0),
	inAckno(0), inWinSize(0),
	maxWinSize(TCP_MAX_WIN), 
	keepAliveTimeout(TCP_ALIVE_TIMEOUT), 
	retransTimeout(TCP_RETRANS_TIMEOUT),
	lastWriteTF(0),lastReadTF(0),
	wcount(0), rcount(0),
	errorState(0),
	/* retranmission variables - init to large */
	rtt_est(TCP_RETRANS_TIMEOUT), 
	rtt_dev(0),
	congestThreshold(TCP_MAX_WIN),
	congestWinSize(MAX_SEG),
	congestUpdate(0),
	udp(lyr)
{
	return;
}

/* Stream Control! */
int	TcpStream::connect()
{
	/* check state */
	if (state != TCP_CLOSED)
	{
		if (state == TCP_ESTABLISHED)
		{
			return 0;
		}
		else if (state < TCP_ESTABLISHED)
		{
			errorState = EAGAIN;
		}
		else
		{
			// major issues!
			errorState = EFAULT;
		}
		return -1;
	}


	/* setup Seqnos */
	outSeqno = genSequenceNo();
	initOurSeqno = outSeqno;

	outAcked = outSeqno; /* min - 1 expected */
	inWinSize = maxWinSize;

	congestThreshold = TCP_MAX_WIN;
	congestWinSize   = MAX_SEG;
	congestUpdate    = outAcked + congestWinSize;

	/* Init Connection */
	/* send syn packet */
	TcpPacket *pkt = new TcpPacket();
	pkt -> setSyn();

#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::connect() Send Init Pkt" << std::endl;
#endif


	/* ********* SLOW START *************
	 * As this is the only place where a syn 
	 * is sent ..... we switch the ttl to 0,
	 * and increment it as we retransmit the packet....
	 * This should help the firewalls along.
	 */

	udp -> setTTL(1);

	toSend(pkt);
	/* change state */
	state = TCP_SYN_SENT;
	std::cerr << "TcpStream STATE -> TCP_SYN_SENT" << std::endl;
	errorState = EAGAIN;
	return -1;
}

/* Stream Control! */
int	TcpStream::close()
{
	cleanup();
	return 0;
}

int	TcpStream::closeWrite()
{
	/* check state */
	/* will always close socket.... */
	/* if in TCP_ESTABLISHED....
	 * -> to state: TCP_FIN_WAIT_1
	 * and shutdown outward stream.
	 */

	/* if in CLOSE_WAIT....
	 * -> to state: TCP_LAST_ACK
	 * and shutdown outward stream.
	 * do this one first!.
	 */ 

	outStreamActive = false;

	if (state == TCP_CLOSE_WAIT)
	{
		/* don't think we need to be
		 * graceful at this point...
		 * connection already closed by other end.
		 * XXX might fix later with scheme
		 *
		 * flag stream closed, and when outqueue
		 * emptied then fin will be sent.
		 */

		/* do nothing */
	}

	if (state == TCP_ESTABLISHED)
	{
		/* fire off the damned thing. */
		/* by changing state */

		/* again this is handled by internals 
		 * the flag however indicates that
		 * no more data can be send, 
		 * and once the queue empties 
		 * the FIN will be sent.
		 */

	}
	if (state == TCP_CLOSED)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::close() Flag Set" << std::endl;
#endif
		return 0;
	}

#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::close() pending" << std::endl;
#endif
	errorState = EAGAIN;
	return -1;
}

int	TcpStream::cleanup()
{
	// This shuts it all down! no matter what.
	
	outStreamActive = false;
	inStreamActive = false;
	state = TCP_CLOSED;
	std::cerr << "TcpStream STATE -> TCP_CLOSED" << std::endl;

	/* Ensure that TTL -> STD for further STUN ATTEMPTS */
	if (udp)
	{
	   udp -> setTTL(TCP_STD_TTL);
	}

	// clear arrays.
	inSize = 0;
	while(inQueue.size() > 0)
	{
		dataBuffer *db = inQueue.front();
		inQueue.pop_front();
		delete db;
	}

	while(outPkt.size() > 0)
	{
		TcpPacket *pkt = outPkt.front();
		outPkt.pop_front();
		delete pkt;
	}


	// clear arrays.
	outSizeRead = 0;
	outSizeNet = 0;
	while(outQueue.size() > 0)
	{
		dataBuffer *db = outQueue.front();
		outQueue.pop_front();
		delete db;
	}

	while(inPkt.size() > 0)
	{
		TcpPacket *pkt = inPkt.front();
		inPkt.pop_front();
		delete pkt;
	}
	return 1;
}

bool	TcpStream::isConnected()
{
	return (state == TCP_ESTABLISHED);
}

int	TcpStream::status(std::ostream &out)
{
	// can leave the timestamp here as time()... rough but okay.
	out << "TcpStream::status @ (" << time(NULL) << ")" << std::endl;
	out << "TcpStream::state = " << (int) state << std::endl;
	out << std::endl;
	out << "writeBuffer: " << inSize << " + 1500 * " << inQueue.size();
	out << " bytes Queued for transmission" << std::endl;
	out << "readBuffer: " << outSizeRead << " + 1500 * ";
	out << outQueue.size() << " + " << outSizeNet;
	out << " incoming bytes waiting" << std::endl;
	out << std::endl;
	out << "inPkts: " << inPkt.size() << " packets waiting for processing";
	out << std::endl;
	out << "outPkts: " << outPkt.size() << " packets waiting for acks";
	out << std::endl;
	out << "us -> peer: nextSeqno: " << outSeqno << " lastAcked: " << outAcked;
	out << " winsize: " << outWinSize;
	out << std::endl;
	out << "peer -> us: Expected SeqNo: " << inAckno;
	out << " winsize: " << inWinSize;
	out << std::endl;
	out << std::endl;
	return state;
}

int     TcpStream::write_allowed()
{
	if (state == TCP_CLOSED)
	{
		errorState = EBADF;
		return -1;
	}
	else if (state < TCP_ESTABLISHED)
	{
		errorState = EAGAIN;
		return -1;
	}
	else if (!outStreamActive)
	{
		errorState = EBADF;
		return -1;
	}

	int maxwrite = (kMaxQueueSize -  inQueue.size()) * MAX_SEG;
	return maxwrite;
}
	

int     TcpStream::read_pending()
{
	/* error should be detected next time */
	int maxread = outSizeRead + outQueue.size() * MAX_SEG + outSizeNet;
	if (state == TCP_CLOSED)
	{
		errorState = EBADF;
		return -1;
	}
	else if (state < TCP_ESTABLISHED)
	{
		errorState = EAGAIN;
		return -1;
	}
	return maxread;
}


	/* stream Interface */
int	TcpStream::write(char *dta, int size) /* write -> pkt -> net */
{

#ifdef DEBUG_TCP_STREAM_EXTRA
static uint32 TMPtotalwrite = 0;
#endif

	if (state == TCP_CLOSED)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::write() Error TCP_CLOSED" << std::endl;
#endif
		errorState = EBADF;
		return -1;
	}
	else if (state < TCP_ESTABLISHED)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::write() Error TCP Not Established" << std::endl;
#endif
		errorState = EAGAIN;
		return -1;
	}
	else if (inQueue.size() > kMaxQueueSize)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::write() Error EAGAIN" << std::endl;
#endif
		errorState = EAGAIN;
		return -1;
	}
	else if (!outStreamActive)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::write() Error TCP_CLOSED" << std::endl;
#endif
		errorState = EBADF;
		return -1;
	}
	
#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::write() = Will Succeed " << size << std::endl;
	std::cerr << "TcpStream::write() Write Start: " << TMPtotalwrite << std::endl;
	std::cerr << printPktOffset(TMPtotalwrite, dta, size) << std::endl;
	TMPtotalwrite += size;
#endif


	if (size + inSize < MAX_SEG)
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::write() Add Itty Bit" << std::endl;
		std::cerr << "TcpStream::write() inData: " << (void *) inData;
		std::cerr << " inSize: " << inSize << " dta: " << (void *) dta;
		std::cerr << " size: " << size << " dest: " << (void *) &(inData[inSize]);
		std::cerr << std::endl;
#endif
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::write() = " << size << std::endl;
#endif
		memcpy((void *) &(inData[inSize]), dta, size);
		inSize += size;
		//std::cerr << "Small Packet - write to net:" << std::endl;
		//std::cerr << printPkt(dta, size) << std::endl;
		return size;
	}

	/* otherwise must construct a dataBuffer.
	 */

#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::write() filling 1 dataBuffer" << std::endl;
	std::cerr << "TcpStream::write() from inData(" << inSize << ")" << std::endl;
	std::cerr << "TcpStream::write() +       dta(" << MAX_SEG - inSize;
	std::cerr << "/" << size << ")" << std::endl;
#endif

	/* first create 1. */
	dataBuffer *db = new dataBuffer;
	memcpy((void *) db->data, (void *) inData, inSize);


	int remSize = size;
	memcpy((void *) &(db->data[inSize]), dta, MAX_SEG - inSize);

	inQueue.push_back(db);
	remSize -= (MAX_SEG - inSize);

#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::write() remaining " << remSize << " bytes to load" << std::endl;
#endif

	while(remSize >= MAX_SEG)
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::write() filling whole dataBuffer" << std::endl;
		std::cerr << "TcpStream::write() from dta[" << size-remSize << "]" << std::endl;
#endif
		db = new dataBuffer;
		memcpy((void *) db->data, (void *) &(dta[size-remSize]), MAX_SEG);

		inQueue.push_back(db);
		remSize -= MAX_SEG;
	}

#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::write() = " << size << std::endl;
#endif

	if (remSize > 0)
	{

#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::write() putting last bit in inData" << std::endl;
		std::cerr << "TcpStream::write() from dta[" << size-remSize << "] size: ";
		std::cerr << remSize << std::endl;
#endif
		memcpy((void *) inData, (void *) &(dta[size-remSize]), remSize);
		inSize = remSize;
		return size;
	}
	else
	{

#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::write() Data fitted exactly in dataBuffer!" << std::endl;
#endif
		inSize = 0;
	}

	return size;
}

int	TcpStream::read(char *dta, int size) /* net ->   pkt -> read */
{
#ifdef DEBUG_TCP_STREAM_EXTRA
static  uint32 TMPtotalread = 0;
#endif
	/* max available data is 
	 * outDataRead + outQueue + outDataNet
	 */

	int maxread = outSizeRead + outQueue.size() * MAX_SEG + outSizeNet;

	if (state == TCP_CLOSED)
	{
		errorState = EBADF;
		return -1;
	}
	else if (state < TCP_ESTABLISHED)
	{
		errorState = EAGAIN;
		return -1;
	}
	else if ((!inStreamActive) && (maxread == 0))
	{
		// finished stream.
		return 0;
	}

	if (maxread == 0)
	{
		/* must wait for more data */
		errorState = EAGAIN;
		return -1;
	}

	if (maxread < size)
	{
#ifdef TCP_NO_PARTIAL_READ
		if (inStreamActive)
		{

#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::read() No Partial Read! ";
			std::cerr << "Can only supply " << maxread << " of ";
			std::cerr << size;
			std::cerr << std::endl;
#endif
			errorState = EAGAIN;
			return -1;
		}
#endif /* TCP_NO_PARTIAL_READ */
		size = maxread;
	}

	/* if less than outDataRead size */
	if (((unsigned) (size) < outSizeRead) && (outSizeRead))
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() Add Itty Bit" << std::endl;
		std::cerr << "TcpStream::read() outSizeRead: " << outSizeRead;
		std::cerr << " size: " << size << " remaining: " << outSizeRead - size;
		std::cerr << std::endl;
#endif
		memcpy(dta,(void *) outDataRead, size);
		memmove((void *) outDataRead, 
		(void *) &(outDataRead[size]), outSizeRead - (size));
		outSizeRead -= size;


#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() = Succeeded " << size << std::endl;
		std::cerr << "TcpStream::read() Read Start: " << TMPtotalread << std::endl;
		std::cerr << printPktOffset(TMPtotalread, dta, size) << std::endl;
#endif

#ifdef DEBUG_TCP_STREAM_EXTRA
		checkData((uint8 *) dta, size, TMPtotalread);
		TMPtotalread += size;
#endif

		/* can allow more in! - update inWinSize */
		UpdateInWinSize();

		return size;
	}

	/* move the whole of outDataRead. */
	if (outSizeRead)
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() Move All outSizeRead" << std::endl;
		std::cerr << "TcpStream::read() outSizeRead: " << outSizeRead;
		std::cerr << " size: " << size;
		std::cerr << std::endl;
#endif
		memcpy(dta,(void *) outDataRead, outSizeRead);

	}

	int remSize = size - outSizeRead;
	outSizeRead = 0;

#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::read() remaining size: " << remSize << std::endl;
#endif

	while((outQueue.size() > 0) && (remSize > 0))
	{
		dataBuffer *db = outQueue.front();
		outQueue.pop_front(); /* remove */

#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() Taking Data from outQueue" << std::endl;
#endif

		/* load into outDataRead */
		if (remSize < MAX_SEG)
		{
#ifdef DEBUG_TCP_STREAM_EXTRA
			std::cerr << "TcpStream::read() Partially using Segment" << std::endl;
			std::cerr << "TcpStream::read() moving: " << remSize << " to dta @: " << size-remSize;
			std::cerr << std::endl;
			std::cerr << "TcpStream::read() rest to outDataRead: " << MAX_SEG - remSize;
			std::cerr << std::endl;
#endif
			memcpy((void *) &(dta[(size)-remSize]), (void *) db->data, remSize);
			memcpy((void *) outDataRead, (void *) &(db->data[remSize]), MAX_SEG - remSize);
			outSizeRead = MAX_SEG - remSize;

			delete db;


#ifdef DEBUG_TCP_STREAM_EXTRA
			std::cerr << "TcpStream::read() = Succeeded " << size << std::endl;
			std::cerr << "TcpStream::read() Read Start: " << TMPtotalread << std::endl;
			std::cerr << printPktOffset(TMPtotalread, dta, size) << std::endl;
#endif

#ifdef DEBUG_TCP_STREAM_EXTRA
			checkData((uint8 *) dta, size, TMPtotalread);
			TMPtotalread += size;
#endif

			/* can allow more in! - update inWinSize */
			UpdateInWinSize();

			return size;
		}
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() Move Whole Segment to dta @ " << size-remSize << std::endl;
#endif

		/* else copy whole segment */
		memcpy((void *) &(dta[(size)-remSize]), (void *) db->data, MAX_SEG);
		remSize -= MAX_SEG;
		delete db;
	}

	/* assumes that outSizeNet >= remSize due to initial 
	 * constraint
	 */
	if ((remSize > 0))
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		std::cerr << "TcpStream::read() Using up : " << remSize;
		std::cerr << " last Bytes, leaving: " << outSizeNet - remSize << std::endl;
#endif


		memcpy((void *) &(dta[(size)-remSize]),(void *) outDataNet, remSize);
		outSizeNet -= remSize;
		if (outSizeNet > 0)
		{
			/* move to the outDataRead */
			memcpy((void *) outDataRead,(void *) &(outDataNet[remSize]), outSizeNet);
			outSizeRead = outSizeNet;
			outSizeNet = 0;
#ifdef DEBUG_TCP_STREAM_EXTRA
			std::cerr << "TcpStream::read() moving last of outSizeNet to outSizeRead: " << outSizeRead;
			std::cerr << std::endl;
#endif

		}


#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::read() = Succeeded " << size << std::endl;
	std::cerr << "TcpStream::read() Read Start: " << TMPtotalread << std::endl;
	std::cerr << printPktOffset(TMPtotalread, dta, size) << std::endl;
#endif

#ifdef DEBUG_TCP_STREAM_EXTRA
		checkData((uint8 *) dta, size, TMPtotalread);
		TMPtotalread += size;
#endif

		/* can allow more in! - update inWinSize */
		UpdateInWinSize();


		return size;
	}

#ifdef DEBUG_TCP_STREAM_EXTRA
	std::cerr << "TcpStream::read() = Succeeded " << size << std::endl;
	std::cerr << "TcpStream::read() Read Start: " << TMPtotalread << std::endl;
	std::cerr << printPktOffset(TMPtotalread, dta, size) << std::endl;
#endif

#ifdef DEBUG_TCP_STREAM_EXTRA
	checkData((uint8 *) dta, size, TMPtotalread);
	TMPtotalread += size;
#endif

	/* can allow more in! - update inWinSize */
	UpdateInWinSize();

	return size;
}


int 	TcpStream::recv()
{
	int  maxsize = MAX_SEG + TCP_PSEUDO_HDR_SIZE;
	int  size = maxsize;
	uint8 input[maxsize];
	double cts = getCurrentTS(); // fractional seconds.


#ifdef DEBUG_TCP_STREAM
	if (state > TCP_SYN_RCVD)
	{
		int availRead = outSizeRead + outQueue.size() * MAX_SEG + outSizeNet;
		std::cerr << "TcpStream::recv() CC: ";
		std::cerr << "  iWS: " << inWinSize;
		std::cerr << "  aRead: " << availRead;
		std::cerr << "  iAck: " << inAckno;
		std::cerr << std::endl;
	}
	else
	{
		std::cerr << "TcpStream::recv() Not Connected";
		std::cerr << std::endl;
	}
#endif


	for(;0 < udp -> readPkt(input, &size); size = maxsize)
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::recv() ReadPkt(" << size << ")" << std::endl;
		//std::cerr << printPkt(input, size);
		//std::cerr << std::endl;
#endif
		TcpPacket *pkt = new TcpPacket();
		if (0 < pkt -> readPacket(input, size))
		{
			lastIncomingPkt = cts;
			handleIncoming(pkt);
		}
		else
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::recv() Bad Packet Deleting!";
			std::cerr << std::endl;
#endif
			delete pkt;
		}
	}

	// make sure we've rcvd something!
	if ((state > TCP_SYN_RCVD) && 
		(cts - lastIncomingPkt > kNoPktTimeout))
	{
		/* shut it all down */
		/* this period should be equivalent
		 * to the firewall timeouts ???
		 *
		 * for max efficiency
		 */

		outStreamActive = false;
		inStreamActive = false;
		state = TCP_CLOSED;
		cleanup();
	}
	return 1;
}

int 	TcpStream::handleIncoming(TcpPacket *pkt)
{
#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::handleIncoming()" << std::endl;
#endif
	switch(state)
	{
		case TCP_CLOSED:
		case TCP_LISTEN:
			/* if receive SYN 
			 * -> respond SYN/ACK
			 * To State: SYN_RCVD
			 *
			 * else Discard.
			 */
			return incoming_Closed(pkt);
			break;
		case TCP_SYN_SENT:
			/* if receive SYN 
			 * -> respond SYN/ACK
			 * To State: SYN_RCVD
			 *
			 * if receive SYN+ACK
			 * -> respond ACK
			 * To State: TCP_ESTABLISHED
			 *
			 * else Discard.
			 */
			return incoming_SynSent(pkt);
			break;
		case TCP_SYN_RCVD:
			/* if receive ACK 
			 * To State: TCP_ESTABLISHED
			 */
			return incoming_SynRcvd(pkt);
			break;
		case TCP_ESTABLISHED:
			/* if receive FIN
			 * -> respond ACK
			 * To State: TCP_CLOSE_WAIT
			 * else Discard.
			 */
			return incoming_Established(pkt);
			break;
		case TCP_FIN_WAIT_1:
			/* state entered by close() call.
			 * if receive FIN
			 * -> respond ACK
			 * To State: TCP_CLOSING
			 *
			 * if receive ACK
			 * -> no response 
			 * To State: TCP_FIN_WAIT_2
			 *
			 * if receive FIN+ACK
			 * -> respond ACK
			 * To State: TCP_TIMED_WAIT
			 *
			 */
			return incoming_Established(pkt);
			//return incoming_FinWait1(pkt);
			break;
		case TCP_FIN_WAIT_2:
			/* if receive FIN
			 * -> respond ACK
			 * To State: TCP_TIMED_WAIT
			 */
			return incoming_Established(pkt);
			//return incoming_FinWait2(pkt);
			break;
		case TCP_CLOSING:
			/* if receive ACK
			 * To State: TCP_TIMED_WAIT
			 */
			/* all handled in Established */
			return incoming_Established(pkt);
			//return incoming_Closing(pkt);
			break;
		case TCP_CLOSE_WAIT:
			/* 
			 * wait for our close to be called.
			 */
			/* all handled in Established */
			return incoming_Established(pkt);
			//return incoming_CloseWait(pkt);
			break;
		case TCP_LAST_ACK:
			/* entered by the local close() after sending FIN.
			 * if receive ACK
			 * To State: TCP_CLOSED
			 */
			/* all handled in Established */
			return incoming_Established(pkt);
			/*
			return incoming_LastAck(pkt);
			 */

			break;
			/* this is actually the only
			 * final state where packets not expected!
			 */
		case TCP_TIMED_WAIT:
			/* State: TCP_TIMED_WAIT
			 *
			 * discard all -> both connections FINed
			 * timeout of this state.
			 *
			 */
			state = TCP_CLOSED;
			// return incoming_TimedWait(pkt);
			break;
	}
	delete pkt;
	return 1;
}


int TcpStream::incoming_Closed(TcpPacket *pkt)
{
	/* if receive SYN 
	 * -> respond SYN/ACK
	 * To State: SYN_RCVD
	 *
	 * else Discard.
	 */

	std::cerr << "TcpStream::incoming_Closed()" << std::endl;
	if ((pkt -> hasSyn()) && (!pkt -> hasAck()))
	{
		/* Init Connection */

		/* save seqno */
		initPeerSeqno = pkt -> seqno;
		inAckno = initPeerSeqno + 1;
		outWinSize = pkt -> winsize;


		inWinSize = maxWinSize;

		/* we can get from SynSent as well, 
		 * but only send one SYN packet 
		 */

		/* start packet */
		TcpPacket *rsp = new TcpPacket();

		if (state == TCP_CLOSED)
		{
			outSeqno = genSequenceNo();
			initOurSeqno = outSeqno;
			outAcked = outSeqno; /* min - 1 expected */

			/* setup Congestion Charging */
			congestThreshold = TCP_MAX_WIN;
			congestWinSize   = MAX_SEG;
			congestUpdate    = outAcked + congestWinSize;

			rsp -> setSyn();
		}
		
		rsp -> setAck(inAckno);
		/* seq + winsize set in toSend() */

		/* as we have received something ... we can up the TTL */
		udp -> setTTL(TCP_STD_TTL);

#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::incoming_Closed() Sending reply" << std::endl;
		std::cerr << "SeqNo: " << rsp->seqno << " Ack: " << rsp->ackno;
		std::cerr << std::endl;
#endif

		toSend(rsp);
		/* change state */
		state = TCP_SYN_RCVD;
		std::cerr << "TcpStream STATE -> TCP_SYN_RCVD" << std::endl;
	}

	delete pkt;
	return 1;
}


int TcpStream::incoming_SynSent(TcpPacket *pkt)
{
	/* if receive SYN 
	 * -> respond SYN/ACK
	 * To State: SYN_RCVD
	 *
	 * if receive SYN+ACK
	 * -> respond ACK
	 * To State: TCP_ESTABLISHED
	 *
	 * else Discard.
	 */

	std::cerr << "TcpStream::incoming_SynSent()" << std::endl;

	if ((pkt -> hasSyn()) && (pkt -> hasAck()))
	{
		/* check stuff */
		if (pkt -> getAck() != outSeqno)
		{
			std::cerr << "TcpStream::incoming_SynSent() Bad Ack - Deleting " << std::endl;
			/* bad ignore */
			delete pkt;
			return -1;
		}

		/* Complete Connection */

		/* save seqno */
		initPeerSeqno = pkt -> seqno;
		inAckno = initPeerSeqno + 1;

		outWinSize = pkt -> winsize;

		outAcked = pkt -> getAck();
	
		/* before ACK, reset the TTL 
		 * As they have sent something, and we have received 
		 * through the firewall, set to STD.
		 */
		udp -> setTTL(TCP_STD_TTL);

		/* ack the Syn Packet */
		sendAck();

		/* change state */
		state = TCP_ESTABLISHED;
		outStreamActive = true;
		inStreamActive = true;

		std::cerr << "TcpStream STATE -> TCP_ESTABLISHED" << std::endl;

		delete pkt;
	} 
	else /* same as if closed! (simultaneous open) */
	{
		return incoming_Closed(pkt);
	}
	return 1;
}


int TcpStream::incoming_SynRcvd(TcpPacket *pkt)
{
	/* if receive ACK 
	 * To State: TCP_ESTABLISHED
	 */
	if (pkt -> hasRst())
	{
		/* trouble */
		state = TCP_CLOSED;
		std::cerr << "TcpStream STATE -> TCP_CLOSED" << std::endl;
		delete pkt;
		return 1;
	}

	bool ackWithData = false;

	if (pkt -> hasAck())
	{
		if (pkt -> hasSyn())
		{
			/* has resent syn -> check it matches */
			std::cerr << "incoming_SynRcvd -> Pkt with ACK + SYN" << std::endl;
		}

		/* check stuff */
		if (pkt -> getAck() != outSeqno)
		{
			/* bad ignore */
			std::cerr << "incoming_SynRcvd -> Ignoring Pkt with bad ACK" << std::endl;
			delete pkt;
			return -1;
		}

		/* Complete Connection */

		/* save seqno */
		if (pkt -> datasize > 0)
		{
			std::cerr << "TcpStream::incoming_SynRcvd() ACK with Data!" << std::endl;
			std::cerr << "TcpStream::incoming_SynRcvd() Shoudn't recv ... unless initACK lost!" << std::endl;
			// managed to trigger this under windows...
			// perhaps the initial Ack was lost, 
			// believe we should just pass this packet
			// directly to the incoming_Established... once
			// the following has been done.
			// and it should all work!
			//exit(1);
			ackWithData = true;
		}

		inAckno = pkt -> seqno; /* + pkt -> datasize; */
		outWinSize = pkt -> winsize;

		outAcked = pkt -> getAck();
		

		/* As they have sent something, and we have received 
		 * through the firewall, set to STD.
		 */
		udp -> setTTL(TCP_STD_TTL);

		/* change state */
		state = TCP_ESTABLISHED;
		outStreamActive = true;
		inStreamActive = true;
		std::cerr << "TcpStream STATE -> TCP_ESTABLISHED" << std::endl;

	} 

	if (ackWithData)
	{
		/* connection Established -> handle normally */
		std::cerr << "incoming_SynRcvd -> Handling Data with Ack Pkt!";
		std::cerr << std::endl;
		incoming_Established(pkt);
	}
	else
	{
		std::cerr << "incoming_SynRcvd -> Ignoring Pkt!" << std::endl;
		/* else nothing */
		delete pkt;
	}
	return 1;
}

int TcpStream::incoming_Established(TcpPacket *pkt)
{
	/* first handle the Ack ... 
	 * this must be done before the queue, 
	 * to keep the values as up-to-date as possible.
	 * 
	 * must sanity check .....
	 * make sure that the sequence number is within the correct range.
	 */

	if ((!isOldSequence(pkt->seqno, inAckno)) &&           // seq >= inAckno
		isOldSequence(pkt->seqno, inAckno + maxWinSize))  // seq < inAckno + maxWinSize.
	{
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::incoming_Established() valid Packet Seqno.";
		std::cerr << std::endl;
#endif
		if (pkt->hasAck())
		{
			outAcked = pkt->ackno;
#ifdef DEBUG_TCP_STREAM
			std::cerr << "\tUpdating OutAcked to: " << outAcked;
			std::cerr << std::endl;
#endif
		}

		outWinSize = pkt->winsize;

#ifdef DEBUG_TCP_STREAM
		std::cerr << "\tUpdating OutWinSize to: " << outWinSize;
		std::cerr << std::endl;
#endif
	}
	else
	{
		/* what we do! */
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::incoming_Established() ERROR out-of-range Packet Seqno.";
		std::cerr << std::endl;
		std::cerr << "TcpStream::incoming_Established() Sending Ack to update Peer";
		std::cerr << std::endl;
#endif

		sendAck();
	}


	/* add to queue */
	inPkt.push_back(pkt);

	if (inPkt.size() > kMaxQueueSize)
	{
		TcpPacket *pkt = inPkt.front();
		inPkt.pop_front();
		delete pkt;

#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::incoming_Established() inPkt reached max size...Discarding Oldest Pkt";
		std::cerr << std::endl;
#endif

	}

	/* use as many packets as possible */
	return check_InPkts();
}

int TcpStream::check_InPkts()
{
	bool found = true;
	TcpPacket *pkt;
	std::list<TcpPacket *>::iterator it;
	while(found)
	{
		found = false;
		for(it = inPkt.begin(); (!found) && (it != inPkt.end());)
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "Checking expInAck: " << inAckno << " vs: " << (*it)->seqno << std::endl;
#endif
			pkt = *it;
			if ((*it)->seqno == inAckno)
			{
				found = true;
				inPkt.erase(it);

			}

			/* see if we can discard it */
			/* if smaller seqno, and not wrapping around */
			else if (isOldSequence((*it)->seqno, inAckno))
			{
#ifdef DEBUG_TCP_STREAM
				std::cerr << "Discarding Old Packet expAck: " << inAckno;
				std::cerr << " seqno: " << (*it)->seqno << std::endl;
#endif

				/* discard */
				it = inPkt.erase(it);
				delete pkt;
				
			}
			else
			{
				it++;
			}
		}
		if (found)
		{

#ifdef DEBUG_TCP_STREAM_EXTRA
			if (pkt->datasize)
			{
				checkData(pkt->data, pkt->datasize, pkt->seqno-initPeerSeqno-1);
			}
#endif

			/* update ack number - let it rollover */
			inAckno = pkt->seqno + pkt->datasize;

			/* XXX This shouldn't be here, as it prevents
			 * the Ack being used until the packet is.
			 * This means that a dropped packet will stop traffic in both 
			 * directions....
			 *
			 * Moved it to incoming_Established .... but extra
			 * check here to be sure!
			 */

			if (pkt->hasAck())
			{
				if (isOldSequence(outAcked, pkt->ackno))
				{
#ifdef DEBUG_TCP_STREAM
					std::cerr << "TcpStream::check_inPkts() ERROR Ack Not Already Used!";
					std::cerr << std::endl;
#endif
					outAcked = pkt->ackno;
					outWinSize = pkt->winsize;

#ifdef DEBUG_TCP_STREAM
					std::cerr << "\tUpdating OutAcked to: " << outAcked;
					std::cerr << std::endl;
					std::cerr << "\tUpdating OutWinSize to: " << outWinSize;
					std::cerr << std::endl;
#endif

				}
				else
				{
#ifdef DEBUG_TCP_STREAM
					std::cerr << "TcpStream::check_inPkts() GOOD Ack Already Used!";
					std::cerr << std::endl;
#endif
				}
			}

			/* push onto queue */

			if (outSizeNet + pkt->datasize < MAX_SEG)
			{
				/* move onto outSizeNet */
				if (pkt->datasize)
				{
				  memcpy((void *) &(outDataNet[outSizeNet]), pkt->data, pkt->datasize);
				  outSizeNet += pkt->datasize;
				}
			}
			else
			{
				/* if it'll overflow the buffer. */
				dataBuffer *db = new dataBuffer();

				/* move outDatNet -> buffer */
				memcpy((void *) db->data, (void *) outDataNet, outSizeNet);

				/* fill rest of space */
				int remSpace = MAX_SEG - outSizeNet;
				memcpy((void *) &(db->data[outSizeNet]), (void *) pkt->data, remSpace);

				/* remove any remaining to outDataNet */
				outSizeNet = pkt->datasize - remSpace;
				if (outSizeNet > 0)
				{
					memcpy((void *) outDataNet, (void *) &(pkt->data[remSpace]), outSizeNet);
				}

				/* push packet onto queue */
				outQueue.push_back(db);
			}

			/* can allow more in! - update inWinSize */
			UpdateInWinSize();

			/* if pkt is FIN */
			/* these must be here -> at the end of the reliable stream */
			/* if the fin is set, ack it specially close stream */
			if (pkt->hasFin())
			{
				/* send final ack */
				sendAck();

				/* closedown stream */
				inStreamActive = false;

				if (state == TCP_ESTABLISHED)
				{
					state = TCP_CLOSE_WAIT;
			std::cerr << "TcpStream::state = TCP_CLOSE_WAIT";
			std::cerr << std::endl;
				}
				else if (state == TCP_FIN_WAIT_1)
				{
					state = TCP_CLOSING;
			std::cerr << "TcpStream::state = TCP_CLOSING";
			std::cerr << std::endl;
				}
				else if (state == TCP_FIN_WAIT_2)
				{
					state = TCP_TIMED_WAIT;
			std::cerr << "TcpStream::state = TCP_TIMED_WAIT";
			std::cerr << std::endl;
					cleanup();
				}
			}

			/* if ack for our FIN */
			if ((pkt->hasAck()) && (!outStreamActive)
				&& (pkt->ackno == outSeqno))
			{
				if (state == TCP_FIN_WAIT_1)
				{
					state = TCP_FIN_WAIT_2;
			std::cerr << "TcpStream::state = TCP_FIN_WAIT_2";
			std::cerr << std::endl;
				}
				else if (state == TCP_LAST_ACK)
				{
					state = TCP_CLOSED;
			std::cerr << "TcpStream::state = TCP_CLOSED";
			std::cerr << std::endl;
					cleanup();
				}
				else if (state == TCP_CLOSING)
				{
					state = TCP_TIMED_WAIT;
			std::cerr << "TcpStream::state = TCP_TIMED_WAIT";
			std::cerr << std::endl;
					cleanup();
				}
			}

			delete pkt;

		} /* end of found */
	} /* while(found) */
	return 1;
}

/* This Fn should be called after each read, or recvd data (thats added to the buffer)
 */
int TcpStream::UpdateInWinSize()
{
	/* InWinSize = maxWinSze - QueuedData, 
	 * actually we can allow a lot more to queue up...
	 * inWinSize = 65536, unless QueuedData > 65536.
	 * 	inWinSize = 2 * maxWinSize - QueuedData;
	 *
	 */

	uint32 queuedData = read_pending();
	if (queuedData < maxWinSize)
	{
		inWinSize = maxWinSize;
	}
	else if (queuedData < 2 * maxWinSize)
	{
		inWinSize = 2 * maxWinSize - queuedData;
	}
	else
	{
		inWinSize = 0;
	}
	return inWinSize;
}

int TcpStream::sendAck()
{
	/* simple -> toSend fills in ack/winsize 
	 * and the rest is history
	 */
#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::sendAck()";
	std::cerr << std::endl;
#endif

	return toSend(new TcpPacket(), false);
}


int TcpStream::toSend(TcpPacket *pkt, bool retrans)
{
	int  outPktSize = MAX_SEG + TCP_PSEUDO_HDR_SIZE;
	char tmpOutPkt[outPktSize];

	/* get accurate timestamp */
	double cts =  getCurrentTS();

	pkt -> winsize = inWinSize;
	pkt -> seqno = outSeqno;

	/* increment seq no */
	if (pkt->datasize)
	{
#ifdef DEBUG_TCP_STREAM_EXTRA
		checkData(pkt->data, pkt->datasize, outSeqno-initOurSeqno-1);
#endif
		outSeqno += pkt->datasize;
	}

	if (pkt->hasSyn()) 
	{
		/* should not have data! */
		if (pkt->datasize)
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "SYN Packet shouldn't contain data!" << std::endl;
#endif
		}
		outSeqno++;
	}
	else
	{
		/* cannot auto Ack SynPackets */
		pkt -> setAck(inAckno);
	}

	pkt -> winsize = inWinSize;

	/* store old info */
	lastSentAck = pkt -> ackno;
	lastSentWinSize = pkt -> winsize;
	keepAliveTimer = cts;
	
	pkt -> writePacket(tmpOutPkt, outPktSize);

#ifdef DEBUG_TCP_STREAM
	std::cerr << "TcpStream::toSend() Seqno: ";
	std::cerr << pkt->seqno << " size: " << pkt->datasize;
	std::cerr << " Ackno: ";
	std::cerr << pkt->ackno << " winsize: " << pkt->winsize;
	std::cerr << std::endl;
	//std::cerr << printPkt(tmpOutPkt, outPktSize) << std::endl;
#endif

	udp -> sendPkt(tmpOutPkt, outPktSize);

	if (retrans)
	{
		/* restart timers */
		pkt -> ts = cts;
		pkt -> retrans = 0;
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::toSend() Adding to outPkt --> Seqno: ";
		std::cerr << pkt->seqno << " size: " << pkt->datasize;
		std::cerr << std::endl;
#endif

		outPkt.push_back(pkt);
	}
	else
	{
		delete pkt;
	}
	return 1;
}



int TcpStream::retrans()
{
	int  outPktSize = MAX_SEG + TCP_PSEUDO_HDR_SIZE;
	char tmpOutPkt[outPktSize];
	bool updateCongestion = true;
	
	/* now retrans */
	double cts =  getCurrentTS();
	std::list<TcpPacket *>::iterator it;
	for(it = outPkt.begin(); (it != outPkt.end()); it++)
	{
		outPktSize = MAX_SEG + TCP_PSEUDO_HDR_SIZE;
		TcpPacket *pkt = (*it);
		if (cts - pkt->ts > retransTimeout) 
		{

			/* retransmission -> adjust the congestWinSize and congestThreshold 
			 * but only once per cycle
			 */
			if (updateCongestion)
			{
			  congestThreshold = congestWinSize / 2;
			  congestWinSize = MAX_SEG;
			  congestUpdate  = outAcked + congestWinSize; // point when we can up the winSize.
			  updateCongestion = false;

#ifdef DEBUG_TCP_STREAM
			  std::cerr << "TcpStream::retrans() Adjusting Congestion Parameters: ";
			  std::cerr << std::endl;
			  std::cerr << "\tcongestWinSize: " << congestWinSize;
			  std::cerr << "  congestThreshold: " << congestThreshold;
			  std::cerr << "  congestUpdate: " << congestUpdate;
			  std::cerr << std::endl;
#endif

			}

			/* before we can retranmit, 
			 * we need to check that its within the congestWinSize
			 * -> actually only checking that the start (seqno) is within window!
			 */


			if (isOldSequence(outAcked + congestWinSize, pkt->seqno))
			{
				/* cannot send .... */
#ifdef DEBUG_TCP_STREAM
				std::cerr << "TcpStream::retrans() Retranmission Delayed by CongestionWindow";
				std::cerr << std::endl;

				std::cerr << "\toutAcked: " << outAcked;
				std::cerr << " CongestWinSize:" << congestWinSize;
				std::cerr << std::endl;

				std::cerr << "\tAttempted Packet: Seqno: ";
				std::cerr << pkt->seqno << " size: " << pkt->datasize;
				std::cerr << " retrans: " << (int) pkt->retrans;
				std::cerr << " timeout: " << retransTimeout;
				std::cerr << std::endl;
#endif
				/* as packets in order, can drop out of the fn now */
				return 0;
			}


#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::retrans() Seqno: ";
			std::cerr << pkt->seqno << " size: " << pkt->datasize;
			std::cerr << " retrans: " << (int) pkt->retrans;
			std::cerr << " timeout: " << retransTimeout;
			std::cerr << std::endl;
#endif

			/* update ackno and winsize */
			if (!(pkt->hasSyn()))
			{
				pkt->setAck(inAckno);
				lastSentAck = pkt -> ackno;
			}

			pkt->winsize = inWinSize;
			lastSentWinSize = pkt -> winsize;

			keepAliveTimer = cts;

			(*it) -> writePacket(tmpOutPkt, outPktSize);

#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::retrans() ReSending Pkt" << std::endl;
			std::cerr << "TcpStream::retrans Seqno: ";
			std::cerr << (*it)->seqno << " size: " << (*it)->datasize;
			std::cerr << " Ackno: ";
			std::cerr << (*it)->ackno << " winsize: " << (*it)->winsize;
			std::cerr << std::endl;
			//std::cerr << printPkt(tmpOutPkt, outPktSize) << std::endl;
#endif
			/* if its a syn packet ** thats been 
			 * transmitting for a while, maybe 
			 * we should increase the ttl.
			 */

			if ((pkt->hasSyn()) && (udp -> getTTL() < TCP_STD_TTL))
			{
				udp -> setTTL(1 + pkt->retrans / 
						TCP_STARTUP_COUNT_PER_TTL);

				std::cerr << "TcpStream::retrans() Startup SYNs";
				std::cerr <<  std::endl;
				std::cerr << "TcpStream::retrans() retransTimeout: ";
				std::cerr <<  retransTimeout << std::endl;

				std::cerr << "TcpStream::retrans() Setting TTL to: ";
				std::cerr << (int) (1 + pkt->retrans / 
					TCP_STARTUP_COUNT_PER_TTL) << std::endl;
			}
			
			/* catch excessive retransmits 
			 * (Allow Syn case more.... )
			 */

			if ((pkt->hasSyn() && (pkt->retrans > kMaxSynPktRetransmit)) ||
				(pkt->retrans > kMaxPktRetransmit))
			{
					/* too many attempts close stream */
#ifdef DEBUG_TCP_STREAM
					std::cerr << "TcpStream::retrans() Too many Retransmission Attempts (";
					std::cerr << (int) pkt->retrans << ") for Pkt" << std::endl;
					std::cerr << "TcpStream::retrans() Closing Socket Connection";
					std::cerr << std::endl;
#endif

					outStreamActive = false;
					inStreamActive = false;
					state = TCP_CLOSED;
					cleanup();
					return 0;
			}


			udp -> sendPkt(tmpOutPkt, outPktSize);

			/* restart timers */
			(*it) -> ts = cts;
			(*it) -> retrans++;	

			/* finally - double the retransTimeout ... (Karn's Algorithm)
			 * this ensures we don't retransmit all the packets that
			 * following a dropped packet!
			 *
			 * but if we have lots of dropped this ain't going to help much!
			 *
			 * not doubling retransTimeout, that is can go manic and result
			 * in excessive timeouts, and no data flow.
			 */
			retransTimeout = 2.0 * (rtt_est + 4.0 * rtt_dev);
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::retrans() Doubling std retranTimeout to:";
			std::cerr << retransTimeout;
			std::cerr << std::endl;
#endif
		}
	}
	return 1;
}


void TcpStream::acknowledge()
{
	/* cleans up acknowledge packets */
	/* packets are pushed back in order */
	std::list<TcpPacket *>::iterator it;
	double cts = getCurrentTS();
	bool updateRTT = true;

	for(it = outPkt.begin(); (it != outPkt.end()) && 
			(isOldSequence((*it)->seqno, outAcked)); 
			it = outPkt.erase(it))
	{
		TcpPacket *pkt = (*it);


		/* adjust the congestWinSize and congestThreshold 
		 * congestUpdate <= outAcked
		 *
		 ***/

		if (!isOldSequence(outAcked, congestUpdate))
		{
			if (congestWinSize < congestThreshold)
			{
				/* double it baby! */
				congestWinSize *= 2;
			}
			else
			{
				/* linear increase */
				congestWinSize += MAX_SEG;
			}

			if (congestWinSize > maxWinSize)
			{
				congestWinSize = maxWinSize;
			}

			congestUpdate  = outAcked + congestWinSize; // point when we can up the winSize.

#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::acknowledge() Adjusting Congestion Parameters: ";
			std::cerr << std::endl;
			std::cerr << "\tcongestWinSize: " << congestWinSize;
			std::cerr << "  congestThreshold: " << congestThreshold;
			std::cerr << "  congestUpdate: " << congestUpdate;
			std::cerr << std::endl;
#endif
		}


		/* update the RoundTripTime, 
		 * using Jacobson's values.
		 * RTT = a RTT + (1-a) M
		 * where
		 * 	RTT is RoundTripTime estimate.
		 * 	a = 7/8, 
		 * 	M = time for ack.
		 *
		 * D = a D + (1 - a) | RTT - M |
		 * where
		 * 	D is approx Deviation.
		 * 	a,RTT & M are the same as above.
		 *
		 * Timeout = RTT + 4 * D.
		 *
		 * And Karn's Algorithm...
		 * which says
		 * 	(1) do not update RTT or D for retransmitted packets.
		 * 		+ the ones that follow .... (the ones whos ack was
		 * 			delayed by the retranmission)
		 * 	(2) double timeout, when packets fail. (done in retrans).
		 */

		if (pkt->retrans)
		{
			updateRTT = false;
		}

		if (updateRTT) /* can use for RTT calc */
		{
			double ack_time = cts - pkt->ts;
			rtt_est = RTT_ALPHA * rtt_est + (1.0 - RTT_ALPHA) * ack_time;
			rtt_dev = RTT_ALPHA * rtt_dev + (1.0 - RTT_ALPHA) * fabs(rtt_est - ack_time);
			retransTimeout = rtt_est + 4.0 * rtt_dev;
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::acknowledge() Updating RTT: ";
			std::cerr << std::endl;
			std::cerr << "\tAckTime: " << ack_time;
			std::cerr << std::endl;
			std::cerr << "\tRRT_est: " << rtt_est;
			std::cerr << std::endl;
			std::cerr << "\tRTT_dev: " << rtt_dev;
			std::cerr << std::endl;
			std::cerr << "\tTimeout: " << retransTimeout;
			std::cerr << std::endl;
#endif
		}

#ifdef DEBUG_TCP_STREAM
		else
		{
			std::cerr << "TcpStream::acknowledge() Not Updating RTT for retransmitted Pkt Sequence";
			std::cerr << std::endl;
		}
#endif

#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::acknowledge() Removing Seqno: ";
		std::cerr << pkt->seqno << " size: " << pkt->datasize;
		std::cerr << std::endl;
#endif
		delete pkt;
	}

	/* This is triggered if we have recieved acks for retransmitted packets....
	 * In this case we want to reset the timeout, and remove the doubling.
	 *
	 * If we don't do this, and there have been more dropped packets, 
	 * the the timeout gets continually doubled. which will virtually stop
	 * all communication.
	 *
	 * This will effectively trigger the retransmission of the next dropped packet.
	 */

	if (!updateRTT)
	{
		retransTimeout = rtt_est + 4.0 * rtt_dev;
	}

	return;
}


/* somehow managed to delete everything.... second time lucky */

int TcpStream::send()
{
	/* handle network interface always */
	/* clean up as much as possible */
	acknowledge();
	/* send any old packets */
	retrans();


	if (state < TCP_ESTABLISHED)
	{
		return -1;
	}

	/* get the inQueue, can send */


	/* determine exactly how much we can send */
	uint32 maxsend = congestWinSize;
	uint32 inTransit;

	if (outWinSize < congestWinSize)
	{
		maxsend = outWinSize;
	}

	if (outSeqno < outAcked)
	{
		inTransit = (TCP_MAX_SEQ - outAcked) + outSeqno;
	}
	else
	{
		inTransit = outSeqno - outAcked;
	}

	if (maxsend > inTransit)
	{
		maxsend -= inTransit;
	}
	else
	{
		maxsend = 0;
	}

#ifdef DEBUG_TCP_STREAM
	int availSend = inQueue.size() * MAX_SEG + inSize;
		std::cerr << "TcpStream::send() CC: ";
		std::cerr << "oWS: " << outWinSize;
		std::cerr << " cWS: " << congestWinSize;
		std::cerr << " | inT: " << inTransit;
		std::cerr << " mSnd: " << maxsend;
		std::cerr << " aSnd: " << availSend;
		std::cerr << " | oSeq: " << outSeqno;
		std::cerr << "  oAck: " << outAcked;
		std::cerr << "  cUpd: " << congestUpdate;
		std::cerr << std::endl;
#endif

	int sent = 0;
	while((inQueue.size() > 0) && (maxsend >= MAX_SEG))
	{
		dataBuffer *db = inQueue.front();
		inQueue.pop_front();

		TcpPacket *pkt = new TcpPacket(db->data, MAX_SEG);
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::send() Segment ===> Seqno: ";
		std::cerr << pkt->seqno << " size: " << pkt->datasize;
		std::cerr << std::endl;
#endif
		sent++;
		maxsend -= MAX_SEG;
		toSend(pkt);
		delete db;
	}

	/* if inqueue empty, and enough window space, send partial stuff */
	if ((!sent) && (inQueue.size() == 0) && (maxsend >= inSize) && (inSize))
	{
		TcpPacket *pkt = new TcpPacket(inData, inSize);
#ifdef DEBUG_TCP_STREAM
		std::cerr << "TcpStream::send() Remaining ===>";
		std::cerr << std::endl;
#endif
		inSize = 0;
		sent++;
		maxsend -= inSize;
		toSend(pkt);
	}

	/* if send nothing */
	bool needsAck = false;

	if (!sent)
	{
		double cts = getCurrentTS();
		/* if needs ack */
		if (isOldSequence(lastSentAck,inAckno))
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::send() Ack Triggered (Ackno)";
			std::cerr << std::endl;
#endif
			needsAck = true;
		}

		/* if needs window 
		 * if added enough space for packet, or
		 * (this case is equivalent to persistence timer)
		 * haven't sent anything for a while, and the
		 * window size has drastically increased.
		 * */
		if (((lastSentWinSize < MAX_SEG) && (inWinSize > MAX_SEG)) ||
		   ((cts - keepAliveTimer > retransTimeout * 4) &&
		   	(inWinSize > lastSentWinSize + 4 * MAX_SEG)))
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::send() Ack Triggered (Window)";
			std::cerr << std::endl;
#endif
			needsAck = true;
		}

		/* if needs keepalive */
		if (cts - keepAliveTimer > keepAliveTimeout)
		{
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::send() Ack Triggered (KAlive)";
			std::cerr << std::endl;
#endif
			needsAck = true;
		}


		/* if end of stream -> switch mode -> send fin (with ack) */
		if ((!outStreamActive) && (inQueue.size() + inSize == 0) &&
			((state == TCP_ESTABLISHED) || (state == TCP_CLOSE_WAIT)))
		{
			/* finish the stream */
			TcpPacket *pkt = new TcpPacket();
			pkt -> setFin();

			needsAck = false;
			toSend(pkt, false);

#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::send() Fin Triggered";
			std::cerr << std::endl;
#endif

			if (state == TCP_ESTABLISHED)
			{
				state = TCP_FIN_WAIT_1;
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::state = TCP_FIN_WAIT_1";
			std::cerr << std::endl;
#endif
			}
			else if (state == TCP_CLOSE_WAIT)
			{
				state = TCP_LAST_ACK;
#ifdef DEBUG_TCP_STREAM
			std::cerr << "TcpStream::state = TCP_LAST_ACK";
			std::cerr << std::endl;
#endif
			}

		}

		if (needsAck)
		{
			sendAck();
		}
	}
	return 1;
}

	





int	TcpStream::tick()
{
	//std::cerr << "TcpStream::tick()" << std::endl;
	recv();
	send();
	return 1;
}

uint32 TcpStream::genSequenceNo()
{
	//return 1000; // TCP_MAX_SEQ - 1000; //1000; //(rand() - 100000) + time(NULL) % 100000;
	return (rand() - 100000) + time(NULL) % 100000;
}


bool 	TcpStream::isOldSequence(uint32 tst, uint32 curr)
{
	return ((int)((tst)-(curr)) < 0);
	
	std::cerr << "TcpStream::isOldSequence(): Case ";
	/* if tst < curr */
	if ((int)((tst)-(curr)) < 0)
	{
		if (curr - tst < TCP_MAX_SEQ/2) /* diff less than half span -> old */
		{
			std::cerr << "1T" << std::endl;
			return true;
		}
		std::cerr << "2F" << std::endl;
		return false;
	}
	else if ((tst - curr) > TCP_MAX_SEQ/2)
	{
		std::cerr << "3T: tst-curr:" << (tst-curr) << std::endl;
		return true;
	}
	std::cerr << "4F: tst-curr:" << (tst-curr) << std::endl;
	return false;
}

#ifdef WINDOWS_SYS
#include <time.h>
#include <sys/timeb.h>
#endif

// Little fn to get current timestamp in an independent manner.
static double getCurrentTS()
{

#ifndef WINDOWS_SYS
        struct timeval cts_tmp;
	gettimeofday(&cts_tmp, NULL);
	double cts =  (cts_tmp.tv_sec) + ((double) cts_tmp.tv_usec) / 1000000.0;
#else
        struct _timeb timebuf;
        _ftime( &timebuf);
	double cts =  (timebuf.time) + ((double) timebuf.millitm) / 1000.0;
#endif
	return cts;
}





static int ilevel = 100;

bool 	TcpStream::widle()
{
	/* init */
	if (!lastWriteTF)
	{
		lastWriteTF = wbytes();
		return false;
	}

	if ((lastWriteTF == wbytes()) && (inSize + inQueue.size() == 0))
	{
		wcount++;
		if (wcount > ilevel)
			return true;
		return false;
	}
	wcount = 0;
	lastWriteTF = wbytes();
	return false;
}


bool 	TcpStream::ridle()
{
	/* init */
	if (!lastReadTF)
	{
		lastReadTF = rbytes();
		return false;
	}

	if ((lastReadTF == rbytes()) && (outSizeRead + outQueue.size() + outSizeNet== 0))
	{
		rcount++;
		if (rcount > ilevel)
			return true;
		return false;
	}
	rcount = 0;
	lastReadTF = rbytes();
	return false;
}

uint32	TcpStream::wbytes()
{
	return outSeqno - initOurSeqno - 1;
}

uint32	TcpStream::rbytes()
{
	return inAckno - initPeerSeqno - 1;
}



/********* Special debugging stuff *****/

#ifdef DEBUG_TCP_STREAM_EXTRA

#include <stdio.h>

static FILE *bc_fd = 0;
int	setupBinaryCheck(std::string fname)
{
	bc_fd = fopen(fname.c_str(), "r");
	return 1;
}

/* uses seq number to track position -> ensure no rollover */
int	checkData(uint8 *data, int size, int idx)
{
	if (bc_fd <= 0)
	{
		return -1;
	}
	std::cerr << "checkData(" << idx << "+" << size << ")";

	int  tmpsize = size;
	uint8 tmpdata[tmpsize];
	if (-1 == fseek(bc_fd, idx, SEEK_SET))
	{
		std::cerr << "Fseek Issues!" << std::endl;
		exit(1);
		return -1;
	}

	if (1 != fread(tmpdata, tmpsize, 1, bc_fd))
	{
		std::cerr << "Length Difference!" << std::endl;
		exit(1);
		return -1;
	}

	for(int i = 0; i < size; i++)
	{
		if (data[i] != tmpdata[i])
		{
			std::cerr << "Byte Difference!" << std::endl;
			exit(1);
			return -1;
		}
	}
	std::cerr << "OK" << std::endl;
	return 1;
}

#endif


