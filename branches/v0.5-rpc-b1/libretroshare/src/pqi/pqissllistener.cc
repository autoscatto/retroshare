/*
 * "$Id: pqissllistener.cc,v 1.3 2007-02-18 21:46:49 rmf24 Exp $"
 *
 * 3P/PQI network interface for RetroShare.
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

#include "pqi/pqissl.h"
#include "pqi/pqissllistener.h"
#include "pqi/pqinetwork.h"
#include "pqi/sslfns.h"

#include "pqi/p3peermgr.h"

#include <errno.h>
#include <openssl/err.h>

#include "util/rsdebug.h"
#include "util/rsstring.h"
#include <unistd.h>

const int pqissllistenzone = 49787;

/* NB: This #define makes the listener open 0.0.0.0:X port instead
 * of a specific port - this might help retroshare work on PCs with
 * multiple interfaces or unique network setups.
 * #define OPEN_UNIVERSAL_PORT 1
 */

#define OPEN_UNIVERSAL_PORT 1

/************************ PQI SSL LISTEN BASE ****************************
 *
 * This provides all of the basic connection stuff, 
 * and calls completeConnection afterwards...
 *
 */


pqissllistenbase::pqissllistenbase(struct sockaddr_in addr, p3PeerMgr *pm)
        :laddr(addr), active(false), mPeerMgr(pm)

{
        if (!(AuthSSL::getAuthSSL()-> active())) {
		pqioutput(PQL_ALERT, pqissllistenzone, 
			"SSL-CTX-CERT-ROOT not initialised!");

		exit(1);
	}

	setuplisten();
	return;
}

pqissllistenbase::~pqissllistenbase()
{
	return;
}

int 	pqissllistenbase::tick()
{
	status();
	// check listen port.
	acceptconnection();
	continueaccepts();
	return finaliseAccepts();
}

int 	pqissllistenbase::status()
{
	std::string out;
	rs_sprintf(out, "pqissllistenbase::status(): Listening on port: %u", ntohs(laddr.sin_port));
	pqioutput(PQL_DEBUG_ALL, pqissllistenzone, out);
	return 1;
}

int	pqissllistenbase::setuplisten()
{
        int err;
	if (active)
		return -1;

        lsock = socket(PF_INET, SOCK_STREAM, 0);
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS // ie UNIX
        if (lsock < 0)
        {
		pqioutput(PQL_ALERT, pqissllistenzone, 
		 "pqissllistenbase::setuplisten() Cannot Open Socket!");

		return -1;
	}

        err = fcntl(lsock, F_SETFL, O_NONBLOCK);
	if (err < 0)
	{
		std::string out;
		rs_sprintf(out, "Error: Cannot make socket NON-Blocking: %d", err);
		pqioutput(PQL_ERROR, pqissllistenzone, out);

		return -1;
	}

/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#else //WINDOWS_SYS 
        if ((unsigned) lsock == INVALID_SOCKET)
        {
        std::string out = "pqissllistenbase::setuplisten() Cannot Open Socket!\n";
        out += "Socket Error: "+ socket_errorType(WSAGetLastError());
        pqioutput(PQL_ALERT, pqissllistenzone, out);

		return -1;
	}

	// Make nonblocking.
	unsigned long int on = 1;
	if (0 != (err = ioctlsocket(lsock, FIONBIO, &on)))
	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::setuplisten() Error: Cannot make socket NON-Blocking: %d\n", err);
		out += "Socket Error: " + socket_errorType(WSAGetLastError());
		pqioutput(PQL_ALERT, pqissllistenzone, out);

		return -1;
	}
#endif
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/

	// setup listening address.

	// fill in fconstant bits.

	laddr.sin_family = AF_INET;

	{
		std::string out = "pqissllistenbase::setuplisten()\n";
		rs_sprintf_append(out, "\tAddress Family: %d\n", (int) laddr.sin_family);
		rs_sprintf_append(out, "\tSetup Address: %s\n", rs_inet_ntoa(laddr.sin_addr).c_str());
		rs_sprintf_append(out, "\tSetup Port: %u", ntohs(laddr.sin_port));

        pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);
                //std::cerr << out.str() << std::endl;
	}
	
	/* added a call to REUSEADDR, so that we can re-open an existing socket
	 * when we restart_listener.
	 */

    	{
      		int on = 1;

/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS // ie UNIX
      		if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
#else //WINDOWS_SYS 
      		if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)) < 0)
#endif 
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
        	{
            std::string out = "pqissllistenbase::setuplisten() Cannot setsockopt SO_REUSEADDR!\n";
			showSocketError(out);
			pqioutput(PQL_ALERT, pqissllistenzone, out);
			std::cerr << out << std::endl;

			exit(1); 
        	}
    	}

#ifdef OPEN_UNIVERSAL_PORT
	struct sockaddr_in tmpaddr = laddr;
	tmpaddr.sin_addr.s_addr = 0;
	if (0 != (err = bind(lsock, (struct sockaddr *) &tmpaddr, sizeof(tmpaddr))))
#else
	if (0 != (err = bind(lsock, (struct sockaddr *) &laddr, sizeof(laddr))))
#endif
	{
		std::string out = "pqissllistenbase::setuplisten()  Cannot Bind to Local Address!\n";
		showSocketError(out);
		pqioutput(PQL_ALERT, pqissllistenzone, out);
		std::cerr << out << std::endl;

		exit(1); 
		return -1;
	}
	else
	{
		pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
		  "pqissllistenbase::setuplisten() Bound to Address.");
	}

#ifdef WINDOWS_SYS
	/* Set TCP buffer size for Windows systems */

	int sockbufsize = 0;
	int size = sizeof(int);

	err = getsockopt(lsock, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize, &size);
	if (err == 0) {
		std::cerr << "pqissllistenbase::setuplisten: Current TCP receive buffer size " << sockbufsize << std::endl;
	} else {
		std::cerr << "pqissllistenbase::setuplisten: Error getting TCP receive buffer size. Error " << err << std::endl;
	}

	sockbufsize = 0;

	err = getsockopt(lsock, SOL_SOCKET, SO_SNDBUF, (char *)&sockbufsize, &size);

	if (err == 0) {
		std::cerr << "pqissllistenbase::setuplisten: Current TCP send buffer size " << sockbufsize << std::endl;
	} else {
		std::cerr << "pqissllistenbase::setuplisten: Error getting TCP send buffer size. Error " << err << std::endl;
	}

	sockbufsize = WINDOWS_TCP_BUFFER_SIZE;

	err = setsockopt(lsock, SOL_SOCKET, SO_RCVBUF, (char *)&sockbufsize, sizeof(sockbufsize));

	if (err == 0) {
		std::cerr << "pqissllistenbase::setuplisten: TCP receive buffer size set to " << sockbufsize << std::endl;
	} else {
		std::cerr << "pqissllistenbase::setuplisten: Error setting TCP receive buffer size. Error " << err << std::endl;
	}

	err = setsockopt(lsock, SOL_SOCKET, SO_SNDBUF, (char *)&sockbufsize, sizeof(sockbufsize));

	if (err == 0) {
		std::cerr << "pqissllistenbase::setuplisten: TCP send buffer size set to " << sockbufsize << std::endl;
	} else {
		std::cerr << "pqissllistenbase::setuplisten: Error setting TCP send buffer size. Error " << err << std::endl;
	}
#endif

	if (0 != (err = listen(lsock, 100)))
	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::setuplisten() Error: Cannot Listen to Socket: %d\n", err);
		showSocketError(out);
		pqioutput(PQL_ALERT, pqissllistenzone, out);
		std::cerr << out << std::endl;

		exit(1); 
		return -1;
	}
	else
	{
		pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
		  "pqissllistenbase::setuplisten() Listening to Socket");
	}
	active = true;
	return 1;
}

int	pqissllistenbase::setListenAddr(struct sockaddr_in addr)
{
	laddr = addr;
	return 1;
}

int	pqissllistenbase::resetlisten()
{
	if (active)
	{
		// close ports etc.
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS // ie UNIX
		shutdown(lsock, SHUT_RDWR);	
		close(lsock);
#else //WINDOWS_SYS 
		closesocket(lsock);
#endif 
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/

		active = false;
		return 1;
	}

	return 0;
}


int	pqissllistenbase::acceptconnection()
{
	if (!active)
		return 0;
	// check port for any socets...
	pqioutput(PQL_DEBUG_ALL, pqissllistenzone, "pqissllistenbase::accepting()");

	// These are local but temp variables...
	// can't be arsed making them all the time.
	struct sockaddr_in remote_addr;
	socklen_t addrlen = sizeof(remote_addr);
	int fd = accept(lsock, (struct sockaddr *) &remote_addr, &addrlen);
	int err = 0;

/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS // ie UNIX
        if (fd < 0)
        {
		pqioutput(PQL_DEBUG_ALL, pqissllistenzone, 
		 "pqissllistenbase::acceptconnnection() Nothing to Accept!");
		return 0;
	}

        err = fcntl(fd, F_SETFL, O_NONBLOCK);
	if (err < 0)
	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::acceptconnection() Error: Cannot make socket NON-Blocking: %d", err);
		pqioutput(PQL_ERROR, pqissllistenzone, out);

		close(fd);
		return -1;
	}

/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#else //WINDOWS_SYS 
        if ((unsigned) fd == INVALID_SOCKET)
        {
		pqioutput(PQL_DEBUG_ALL, pqissllistenzone, 
		 "pqissllistenbase::acceptconnnection() Nothing to Accept!");
		return 0;
	}

	// Make nonblocking.
	unsigned long int on = 1;
	if (0 != (err = ioctlsocket(fd, FIONBIO, &on)))
	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::acceptconnection() Error: Cannot make socket NON-Blocking: %d\n", err);
		out += "Socket Error:" + socket_errorType(WSAGetLastError());
		pqioutput(PQL_ALERT, pqissllistenzone, out);

		closesocket(fd);
		return 0;
	}
#endif
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/

	{
	  std::string out;
	  rs_sprintf(out, "Accepted Connection from %s:%u", rs_inet_ntoa(remote_addr.sin_addr).c_str(), ntohs(remote_addr.sin_port));
	  pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);
	}

	// Negotiate certificates. SSL stylee.
	// Allow negotiations for secure transaction.
	
	IncomingSSLInfo incoming_connexion_info ;

	incoming_connexion_info.ssl   = SSL_new(AuthSSL::getAuthSSL() -> getCTX());
	incoming_connexion_info.addr  = remote_addr ;
	incoming_connexion_info.gpgid = "" ;
	incoming_connexion_info.sslid = "" ;
	incoming_connexion_info.sslcn = "" ;

	SSL_set_fd(incoming_connexion_info.ssl, fd);

	return continueSSL(incoming_connexion_info, true); // continue and save if incomplete.
}

int	pqissllistenbase::continueSSL(IncomingSSLInfo& incoming_connexion_info, bool addin)
{
	// attempt the accept again.
	int fd =  SSL_get_fd(incoming_connexion_info.ssl);

	// clear the connexion info that will be filled in by the callback.
	//
	AuthSSL::getAuthSSL()->setCurrentConnectionAttemptInfo(std::string(),std::string(),std::string()) ;

	int err = SSL_accept(incoming_connexion_info.ssl);

	// No grab the connexion info that was filled in by the callback.
	//
	AuthSSL::getAuthSSL()->getCurrentConnectionAttemptInfo(incoming_connexion_info.gpgid,incoming_connexion_info.sslid,incoming_connexion_info.sslcn) ;

	if (err <= 0)
	{
		int ssl_err = SSL_get_error(incoming_connexion_info.ssl, err);
		int err_err = ERR_get_error();

		{
			std::string out;
			rs_sprintf(out, "pqissllistenbase::continueSSL() Issues with SSL Accept(%d)!\n", err);
			printSSLError(incoming_connexion_info.ssl, err, ssl_err, err_err, out);
			pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);
		}

		switch (ssl_err) {
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			{
				std::string out = "pqissllistenbase::continueSSL() Connection Not Complete!\n";

				if (addin)
				{
					out += "pqissllistenbase::continueSSL() Adding SSL to incoming!";

					// add to incomingqueue.
					incoming_ssl.push_back(incoming_connexion_info) ;
				}

				pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);

				// zero means still continuing....
				return 0;
			}
			break;
			case SSL_ERROR_SYSCALL:
			{
				std::string out = "pqissllistenbase::continueSSL() Connection failed!\n";
				pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);

				closeConnection(fd, incoming_connexion_info.ssl);

				// basic-error while connecting, no security message needed
				return -1;
			}
			break;
		}

		/* we have failed -> get certificate if possible */
		Extract_Failed_SSL_Certificate(incoming_connexion_info);

		closeConnection(fd, incoming_connexion_info.ssl) ;

		pqioutput(PQL_WARNING, pqissllistenzone, "Read Error on the SSL Socket\nShutting it down!");

		// failure -1, pending 0, sucess 1.
		return -1;
	}
	
	// if it succeeds
	if (0 < completeConnection(fd, incoming_connexion_info))
	{
		return 1;
	}

	/* else we shut it down! */
  	pqioutput(PQL_WARNING, pqissllistenzone, 
	 	"pqissllistenbase::completeConnection() Failed!");

	closeConnection(fd, incoming_connexion_info.ssl) ;

	pqioutput(PQL_WARNING, pqissllistenzone, "Shutting it down!");

	// failure -1, pending 0, sucess 1.
	return -1;
}


int pqissllistenbase::closeConnection(int fd, SSL *ssl)
{
	/* else we shut it down! */
  	pqioutput(PQL_WARNING, pqissllistenzone, "pqissllistenbase::closeConnection() Shutting it Down!");

	// delete ssl connection.
	SSL_shutdown(ssl);

	// close socket???
/************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS // ie UNIX
	shutdown(fd, SHUT_RDWR);	
	close(fd);
#else //WINDOWS_SYS 
	closesocket(fd);
#endif 
/************************** WINDOWS/UNIX SPECIFIC PART ******************/
	// free connection.
	SSL_free(ssl);

	return 1;
}




int 	pqissllistenbase::Extract_Failed_SSL_Certificate(const IncomingSSLInfo& info)
{
  	pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, "pqissllistenbase::Extract_Failed_SSL_Certificate()");

	std::cerr << "pqissllistenbase::Extract_Failed_SSL_Certificate() FAILED CONNECTION due to security!";
	std::cerr << std::endl;

	// Get the Peer Certificate....
	X509 *peercert = SSL_get_peer_certificate(info.ssl);

	std::cerr << "Extract_Failed_SSL_Certificate: " << std::endl;
	std::cerr << "   SSL    = " << (void*)info.ssl << std::endl;
	std::cerr << "   GPG id = " << info.gpgid << std::endl;
	std::cerr << "   SSL id = " << info.sslid << std::endl;
	std::cerr << "   SSL cn = " << info.sslcn << std::endl;
	std::cerr << "   addr+p = " << rs_inet_ntoa(info.addr.sin_addr) << ":" <<  ntohs(info.addr.sin_port) << std::endl;

	if (peercert == NULL)
	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::Extract_Failed_SSL_Certificate() from: %s:%u ERROR Peer didn't give Cert!", rs_inet_ntoa(info.addr.sin_addr).c_str(), ntohs(info.addr.sin_port));
		std::cerr << out << std::endl;
        AuthSSL::getAuthSSL()->FailedCertificate(peercert, info.gpgid,info.sslid,info.sslcn,info.addr, true);

		pqioutput(PQL_WARNING, pqissllistenzone, out);
		return -1;
	}

  	pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
	  "pqissllistenbase::Extract_Failed_SSL_Certificate() Have Peer Cert - Registering");

	{
		std::string out;
		rs_sprintf(out, "pqissllistenbase::Extract_Failed_SSL_Certificate() from: %s:%u Passing Cert to AuthSSL() for analysis", rs_inet_ntoa(info.addr.sin_addr).c_str(), ntohs(info.addr.sin_port));
		std::cerr << out << std::endl;

		pqioutput(PQL_WARNING, pqissllistenzone, out);
		std::cerr << out << std::endl;
	}

	// save certificate... (and ip locations)
	// false for outgoing....
	AuthSSL::getAuthSSL()->FailedCertificate(peercert, info.gpgid,info.sslid,info.sslcn,info.addr, true);

	return 1;
}


int	pqissllistenbase::continueaccepts()
{

	// for each of the incoming sockets.... call continue.

	for(std::list<IncomingSSLInfo>::iterator it = incoming_ssl.begin(); it != incoming_ssl.end();)
	{
		pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, "pqissllistenbase::continueaccepts() Continuing SSL");

		if (0 != continueSSL( *it, false))
		{
			pqioutput(PQL_DEBUG_ALERT, pqissllistenzone, 
					"pqissllistenbase::continueaccepts() SSL Complete/Dead!");

			/* save and increment -> so we can delete */
			std::list<IncomingSSLInfo>::iterator itd = it++;
			incoming_ssl.erase(itd);
		}
		else
			it++;
	}
	return 1;
}

#define ACCEPT_WAIT_TIME 30	

int	pqissllistenbase::finaliseAccepts()
{

	// for each of the incoming sockets.... call continue.
	std::list<AcceptedSSL>::iterator it;

	time_t now = time(NULL);
	for(it = accepted_ssl.begin(); it != accepted_ssl.end();)
	{
  	        pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
		  "pqissllistenbase::finalisedAccepts() Continuing SSL Accept");

		/* check that the socket is still active - how? */
		int active = isSSLActive(it->mFd, it->mSSL);
		if (active > 0)
		{
  	        	pqioutput(PQL_WARNING, pqissllistenzone, 
		  		"pqissllistenbase::finaliseAccepts() SSL Connection Ok => finaliseConnection");

			if (0 > finaliseConnection(it->mFd, it->mSSL, it->mPeerId, it->mAddr))
			{
				closeConnection(it->mFd, it->mSSL);
			}
			it = accepted_ssl.erase(it);
		}
		else if (active < 0)
		{
          		pqioutput(PQL_WARNING, pqissllistenzone, 
		  		"pqissllistenbase::finaliseAccepts() SSL Connection Dead => closeConnection");

			closeConnection(it->mFd, it->mSSL);
			it = accepted_ssl.erase(it);
		}
		else if (now - it->mAcceptTS > ACCEPT_WAIT_TIME)
		{
          		pqioutput(PQL_WARNING, pqissllistenzone, 
		  		"pqissllistenbase::finaliseAccepts() SSL Connection Timed Out => closeConnection");
			closeConnection(it->mFd, it->mSSL);
			it = accepted_ssl.erase(it);
		}
		else
		{
          		pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
		  		"pqissllistenbase::finaliseAccepts() SSL Connection Status Unknown");
			it++;
		}
	}
	return 1;
}

int pqissllistenbase::isSSLActive(int /*fd*/, SSL *ssl)
{

	/* can we just get error? */
	int bufsize = 8; /* just a little look */
	uint8_t buf[bufsize];
	int err = SSL_peek(ssl, buf, bufsize);
	if (err <= 0)
	{
		int ssl_err = SSL_get_error(ssl, err);
		int err_err = ERR_get_error();

		{
		  std::string out;
		  rs_sprintf(out, "pqissllistenbase::isSSLActive() Issues with SSL_Accept(%d)!\n", err);
		  printSSLError(ssl, err, ssl_err, err_err, out);
		  pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);
		}

		if (ssl_err == SSL_ERROR_ZERO_RETURN)
		{
			pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, "pqissllistenbase::isSSLActive() SSL_ERROR_ZERO_RETURN Connection state unknown");

			// zero means still continuing....
			return 0;
		}
		if ((ssl_err == SSL_ERROR_WANT_READ) || 
		   (ssl_err == SSL_ERROR_WANT_WRITE))
		{
			pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, "pqissllistenbase::isSSLActive() SSL_ERROR_WANT_READ || SSL_ERROR_WANT_WRITE Connection state unknown");

			// zero means still continuing....
			return 0;
		}
		else
		{
			std::string out;
			rs_sprintf(out, "pqissllistenbase::isSSLActive() Issues with SSL Peek(%d) Likely the Connection was killed by Peer\n", err);
			printSSLError(ssl, err, ssl_err, err_err, out);
			pqioutput(PQL_ALERT, pqissllistenzone, out);

			return -1;
		}
	}

	pqioutput(PQL_WARNING, pqissllistenzone, "pqissllistenbase::isSSLActive() Successful Peer -> Connection Okay");

	return 1;
}

/************************ PQI SSL LISTENER ****************************
 *
 * This is the standard pqissl listener interface....
 *
 * this class only allows connections from 
 * specific certificates, which are pre specified.
 *
 */

pqissllistener::pqissllistener(struct sockaddr_in addr, p3PeerMgr *lm)
        :pqissllistenbase(addr, lm)
{
	return;
}

pqissllistener::~pqissllistener()
{
	return;
}

int 	pqissllistener::addlistenaddr(std::string id, pqissl *acc)
{
	std::map<std::string, pqissl *>::iterator it;

	std::string out = "Adding to Cert Listening Addresses Id: " + id + "\nCurrent Certs:\n";
	for(it = listenaddr.begin(); it != listenaddr.end(); it++)
	{
		out += id + "\n";
		if (it -> first == id)
		{
			out += "pqissllistener::addlistenaddr() Already listening for Certificate!\n";
			
			pqioutput(PQL_DEBUG_ALERT, pqissllistenzone, out);
			return -1;

		}
	}

	pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);

	// not there can accept it!
	listenaddr[id] = acc;
	return 1;
}

int	pqissllistener::removeListenPort(std::string id)
{
	// check where the connection is coming from.
	// if in list of acceptable addresses, 
	//
	// check if in list.
	std::map<std::string, pqissl *>::iterator it;
	for(it = listenaddr.begin();it!=listenaddr.end();it++)
	{
		if (it->first == id)
		{
			listenaddr.erase(it);

  	        	pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, 
			  "pqissllisten::removeListenPort() Success!");
			return 1;
		}
	}

  	pqioutput(PQL_WARNING, pqissllistenzone, 
	  "pqissllistener::removeListenPort() Failed to Find a Match");

	return -1;
}


int 	pqissllistener::status()
{
	pqissllistenbase::status();
	// print certificates we are listening for.
	std::map<std::string, pqissl *>::iterator it;

	std::string out = "pqissllistener::status(): ";
	rs_sprintf(out, " Listening (%u) for Certs:", ntohs(laddr.sin_port));
	for(it = listenaddr.begin(); it != listenaddr.end(); it++)
	{
		out += "\n" + it -> first ;
	}
	pqioutput(PQL_DEBUG_ALL, pqissllistenzone, out);

	return 1;
}

int pqissllistener::completeConnection(int fd, IncomingSSLInfo& info)
{ 

	// Get the Peer Certificate....
	X509 *peercert = SSL_get_peer_certificate(info.ssl);

	if (peercert == NULL)
	{
  	        pqioutput(PQL_WARNING, pqissllistenzone, 
		 "pqissllistener::completeConnection() Peer Did Not Provide Cert!");

		// failure -1, pending 0, sucess 1.
		// pqissllistenbase will shutdown!
		return -1;
	}

	// Check cert.
	std::string newPeerId;


	/****
	 * As the validation is actually done before this...
	 * we should only need to call CheckCertificate here!
	 ****/

        bool certOk = AuthSSL::getAuthSSL()->ValidateCertificate(peercert, newPeerId);

	bool found = false;
	std::map<std::string, pqissl *>::iterator it;

	// Let connected one through as well! if ((npc == NULL) || (npc -> Connected()))
	if (!certOk)
	{
  	        pqioutput(PQL_WARNING, pqissllistenzone, 
		 "pqissllistener::completeConnection() registerCertificate Failed!");

		// bad - shutdown.
		// pqissllistenbase will shutdown!
		X509_free(peercert);

		return -1;
	}
	else
	{
		std::string out = "pqissllistener::continueSSL()\nchecking: " + newPeerId + "\n";
		// check if cert is in our list.....
		for(it = listenaddr.begin();(found!=true) && (it!=listenaddr.end());)
		{
			out + "\tagainst: " + it->first + "\n";
			if (it -> first == newPeerId)
			{
				// accept even if already connected.
				out += "\t\tMatch!";
				found = true;
			}
			else
			{
				it++;
			}
		}

		pqioutput(PQL_DEBUG_BASIC, pqissllistenzone, out);
	}
	
	if (found == false)
	{
		std::string out = "No Matching Certificate for Connection:" + rs_inet_ntoa(info.addr.sin_addr) +"\npqissllistenbase: Will shut it down!";
		pqioutput(PQL_WARNING, pqissllistenzone, out);

		// but as it passed the authentication step, 
		// we can add it into the AuthSSL, and mConnMgr.

		AuthSSL::getAuthSSL()->CheckCertificate(newPeerId, peercert);

		/* now need to get GPG id too */
		std::string pgpid = getX509CNString(peercert->cert_info->issuer);
		mPeerMgr->addFriend(newPeerId, pgpid);
	
		X509_free(peercert);
		return -1;
	}

	// Cleanup cert.
	X509_free(peercert);

	// Pushback into Accepted List.
	AcceptedSSL as;
	as.mFd = fd;
	as.mSSL = info.ssl;
	as.mPeerId = newPeerId;
	as.mAddr = info.addr;
	as.mAcceptTS = time(NULL);

	accepted_ssl.push_back(as);

	std::string out = "pqissllistener::completeConnection() Successful Connection with: " + newPeerId;
	out += " for Connection:" + rs_inet_ntoa(info.addr.sin_addr) + " Adding to WAIT-ACCEPT Queue";
	pqioutput(PQL_WARNING, pqissllistenzone, out);

	return 1;
}

int pqissllistener::finaliseConnection(int fd, SSL *ssl, std::string peerId, struct sockaddr_in &remote_addr)
{ 
	std::map<std::string, pqissl *>::iterator it;

	std::string out = "pqissllistener::finaliseConnection()\n";
	out += "checking: " + peerId + "\n";
	// check if cert is in the list.....

	it = listenaddr.find(peerId);
	if (it == listenaddr.end())
	{
		out += "No Matching Peer for Connection:" + rs_inet_ntoa(remote_addr.sin_addr);
		out += "\npqissllistener => Shutting Down!";
		pqioutput(PQL_WARNING, pqissllistenzone, out);
		return -1;
	}

	out += "Found Matching Peer for Connection:" + rs_inet_ntoa(remote_addr.sin_addr);
	out += "\npqissllistener => Passing to pqissl module!";
	pqioutput(PQL_WARNING, pqissllistenzone, out);

	// hand off ssl conection.
	pqissl *pqis = it -> second;
	pqis -> accept(ssl, fd, remote_addr);

	return 1;
}
