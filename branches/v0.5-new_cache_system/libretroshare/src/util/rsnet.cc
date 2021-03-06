/*
 * libretroshare/src/util: rsnet.cc
 *
 * Universal Networking Header for RetroShare.
 *
 * Copyright 2007-2008 by Robert Fernie.
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
#include "util/rsthreads.h"
#include <string.h>
#include <sstream>

#ifdef WINDOWS_SYS
#else
#include <netdb.h>
#endif

/* enforce LITTLE_ENDIAN on Windows */
#ifdef WINDOWS_SYS
	#define BYTE_ORDER  1234
	#define LITTLE_ENDIAN 1234
	#define BIG_ENDIAN  4321
#endif

uint64_t ntohll(uint64_t x)
{
#ifdef BYTE_ORDER
        #if BYTE_ORDER == BIG_ENDIAN
                return x;
        #elif BYTE_ORDER == LITTLE_ENDIAN

                uint32_t top = (uint32_t) (x >> 32);
                uint32_t bot = (uint32_t) (0x00000000ffffffffULL & x);

                uint64_t rev = ((uint64_t) ntohl(top)) | (((uint64_t) ntohl(bot)) << 32);

                return rev;
        #else
                #error "ENDIAN determination Failed"
        #endif
#else
        #error "ENDIAN determination Failed (BYTE_ORDER not defined)"
#endif

}

uint64_t htonll(uint64_t x)
{
        return ntohll(x);
}

void sockaddr_clear(struct sockaddr_in *addr)
{
        memset(addr, 0, sizeof(struct sockaddr_in));
        addr->sin_family = AF_INET;
}


bool    isValidNet(const struct in_addr *addr)
{
        // invalid address.
	if((*addr).s_addr == INADDR_NONE)
		return false;
	if((*addr).s_addr == 0)
		return false;
	// should do more tests.
	return true;
}


bool    isLoopbackNet(const struct in_addr *addr)
{
	in_addr_t taddr = ntohl(addr->s_addr);
	return (taddr == (127 << 24 | 1));
}

bool    isPrivateNet(const struct in_addr *addr)
{
	in_addr_t taddr = ntohl(addr->s_addr);

	// 10.0.0.0/8
	// 172.16.0.0/12
	// 192.168.0.0/16
	// 169.254.0.0/16
	if ((taddr>>24 == 10) ||
 		(taddr>>20 == (172<<4 | 16>>4)) ||
 		(taddr>>16 == (192<<8 | 168)) ||
 		(taddr>>16 == (169<<8 | 254)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool    isExternalNet(const struct in_addr *addr)
{
	if (!isValidNet(addr))
	{
		return false;
	}
	if (isLoopbackNet(addr))
	{
		return false;
	}
	if (isPrivateNet(addr))
	{
		return false;
	}
	return true;
}

std::ostream &operator<<(std::ostream &out, const struct sockaddr_in &addr)
{
	out << "[" << inet_ntoa(addr.sin_addr) << ":";
	out << htons(addr.sin_port) << "]";
	return out;
}

/* thread-safe version of inet_ntoa */
/*** XXX, PROBLEM this function is not Thread-Safe.
 * because it can be called in lots of other parts of the program.
 * which could still collide with this one!
 *
 * Must rewrite to make truely thread-safe.
 */

std::string rs_inet_ntoa(struct in_addr in)
{
	std::ostringstream str;
	uint8_t *bytes = (uint8_t *) &(in.s_addr);
	str << (int) bytes[0] << ".";
	str << (int) bytes[1] << ".";
	str << (int) bytes[2] << ".";
	str << (int) bytes[3];
	return str.str();
}

