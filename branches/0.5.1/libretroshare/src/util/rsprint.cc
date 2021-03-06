
/*
 * libretroshare/src/util: rsprint.cc
 *
 * RetroShare Utilities
 *
 * Copyright 2008 by Robert Fernie.
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

#include "util/rsprint.h"
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

std::string RsUtil::BinToHex(std::string bin)
{
	return BinToHex(bin.c_str(), bin.length());
}

std::string RsUtil::BinToHex(const char *arr, const uint32_t len)
{
        std::ostringstream out;

        for(uint32_t j = 0; j < len; j++)
        {
		out << std::hex << std::setw(2) << std::setfill('0') << (int32_t) (((const uint8_t *) arr)[j]);
        }
        return out.str();
}


std::string RsUtil::HashId(std::string id, bool reverse)
{
	std::string hash;
	std::string tohash;
	if (reverse)
	{	
		std::string::reverse_iterator rit;
		for(rit = id.rbegin(); rit != id.rend(); rit++)
		{
			tohash += (*rit);
		}
	}
	else
	{
		tohash = id;
	}

	SHA_CTX *sha_ctx = new SHA_CTX;
	uint8_t  sha_hash[SHA_DIGEST_LENGTH];

	SHA1_Init(sha_ctx);
	SHA1_Update(sha_ctx, tohash.c_str(), tohash.length());
	SHA1_Final(sha_hash, sha_ctx);

	for(uint16_t i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		hash += sha_hash[i];
	}

	/* cleanup */
	delete sha_ctx;

	return hash;
}




#ifdef WINDOWS_SYS
#include <time.h>
#include <sys/timeb.h>
#endif

#include <sys/time.h>
#include <sstream>
#include <iomanip>

static double getCurrentTS();

// Little fn to get current timestamp in an independent manner.
std::string RsUtil::AccurateTimeString()
{
	std::ostringstream out;
	out << std::setprecision(15) << getCurrentTS();
	return out.str();
}

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




