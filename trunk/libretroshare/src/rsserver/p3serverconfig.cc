/*
 * libretroshare/src/rsserver: p3serverconfig.h
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2011-2011 by Robert Fernie.
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

#include "rsserver/p3serverconfig.h"
#include "services/p3bwctrl.h"

RsServerConfig *rsConfig = NULL;


p3ServerConfig::p3ServerConfig(p3PeerMgr *peerMgr, p3LinkMgr *linkMgr, p3NetMgr *netMgr, p3GeneralConfig *genCfg)
:configMtx("p3ServerConfig")
{
	mPeerMgr = peerMgr;
	mLinkMgr = linkMgr;
	mNetMgr = netMgr;

	mGeneralConfig = genCfg;

	mUserLevel = RSCONFIG_USER_LEVEL_NEW; /* START LEVEL */

	rsConfig = this;
}


p3ServerConfig::~p3ServerConfig() 
{ 
	return; 
}


#define RS_CONFIG_ADVANCED_STRING 	"AdvMode"

bool p3ServerConfig::findConfigurationOption(uint32_t key, std::string &keystr)
{
	bool found = false;
	switch(key)
	{
		case RS_CONFIG_ADVANCED:
			keystr = RS_CONFIG_ADVANCED_STRING;
			found = true;
			break;
	}
	return found;
}


bool p3ServerConfig::getConfigurationOption(uint32_t key, std::string &opt)
{
	std::string strkey;
	if (!findConfigurationOption(key, strkey))
	{
		std::cerr << "p3ServerConfig::getConfigurationOption() OPTION NOT VALID: " << key;
		std::cerr << std::endl;
		return false;
	}

	opt = mGeneralConfig->getSetting(strkey);
	return true;
}

		
bool p3ServerConfig::setConfigurationOption(uint32_t key, const std::string &opt)
{
	std::string strkey;
	if (!findConfigurationOption(key, strkey))
	{
		std::cerr << "p3ServerConfig::setConfigurationOption() OPTION NOT VALID: " << key;
		std::cerr << std::endl;
		return false;
	}

	mGeneralConfig->setSetting(strkey, opt);
	return true;
}


	/* From RsIface::RsConfig */

int 	p3ServerConfig::getConfigNetStatus(RsConfigNetStatus &/*status*/)
{
	return 0;
}

int 	p3ServerConfig::getConfigStartup(RsConfigStartup &/*params*/)
{
	return 0;
}

#if 0
int 	p3ServerConfig::getConfigDataRates(RsConfigDataRates &/*params*/)
{
	return 0;
}
#endif

                /***** for RsConfig -> p3BandwidthControl ****/

int 	p3ServerConfig::getTotalBandwidthRates(RsConfigDataRates &rates)
{
	if (rsBandwidthControl)
	{
		return rsBandwidthControl->getTotalBandwidthRates(rates);
	}
	return 0;
}


int 	p3ServerConfig::getAllBandwidthRates(std::map<std::string, RsConfigDataRates> &ratemap)
{
	if (rsBandwidthControl)
	{
		return rsBandwidthControl->getAllBandwidthRates(ratemap);
	}
	return 0;
}


	/* From RsInit */

std::string      p3ServerConfig::RsConfigDirectory()
{
	return std::string();
}

std::string      p3ServerConfig::RsConfigKeysDirectory()
{
	return std::string();
}


std::string  p3ServerConfig::RsProfileConfigDirectory()
{
	return std::string();
}

bool         p3ServerConfig::getStartMinimised()
{
	return 0;
}

std::string  p3ServerConfig::getRetroShareLink()
{
	return std::string();
}


bool p3ServerConfig::getAutoLogin()
{
	return 0;
}

void p3ServerConfig::setAutoLogin(bool /*autoLogin*/)
{
	return;
}

bool p3ServerConfig::RsClearAutoLogin()
{
	return 0;
}


std::string p3ServerConfig::getRetroshareDataDirectory()
{
	return std::string();
}

	/* New Stuff */

uint32_t p3ServerConfig::getUserLevel()
{
	uint32_t userLevel = RSCONFIG_USER_LEVEL_NEW;
	{
		RsStackMutex stack(configMtx); /******* LOCKED MUTEX *****/
		userLevel = mUserLevel;
	}

	switch(userLevel)
	{
                case RSCONFIG_USER_LEVEL_OVERRIDE:
			break;

#define MIN_BASIC_FRIENDS 2
			
		// FALL THROUGH EVERYTHING.
		default:
		case RSCONFIG_USER_LEVEL_NEW:
		{

			if (mPeerMgr->getFriendCount(true, false) > MIN_BASIC_FRIENDS)
			{
				userLevel = RSCONFIG_USER_LEVEL_BASIC;
			}
		}
		case RSCONFIG_USER_LEVEL_BASIC:
		{
			/* check that we have some lastConnect > 0 */
			if (mPeerMgr->haveOnceConnected())
			{
				userLevel = RSCONFIG_USER_LEVEL_CASUAL;
			}
		}

		case RSCONFIG_USER_LEVEL_CASUAL:
		case RSCONFIG_USER_LEVEL_POWER:

		{
			/* check that the firewall is open */

			uint32_t netMode = mNetMgr->getNetworkMode();
			uint32_t firewallMode = mNetMgr->getNatHoleMode();

			if ((RSNET_NETWORK_EXTERNALIP == netMode) ||
			   ((RSNET_NETWORK_BEHINDNAT == netMode) &&
			  	 	((RSNET_NATHOLE_UPNP == firewallMode) ||
			   		(RSNET_NATHOLE_NATPMP == firewallMode) ||
			   		(RSNET_NATHOLE_FORWARDED == firewallMode))))
			{
				userLevel = RSCONFIG_USER_LEVEL_POWER;
			}
		}
			break; /* for all */
	}

	{
		RsStackMutex stack(configMtx); /******* LOCKED MUTEX *****/
		mUserLevel = userLevel;
	}

	return userLevel;
}


uint32_t p3ServerConfig::getNetState()
{
	return mNetMgr->getNetStateMode();
}

uint32_t p3ServerConfig::getNetworkMode()
{
	return mNetMgr->getNetworkMode();
}

uint32_t p3ServerConfig::getNatTypeMode()
{
	return mNetMgr->getNatTypeMode();
}

uint32_t p3ServerConfig::getNatHoleMode()
{
	return mNetMgr->getNatHoleMode();
}

uint32_t p3ServerConfig::getConnectModes()
{
	return mNetMgr->getConnectModes();
}

        /* Operating Mode */
#define RS_CONFIG_OPERATING_STRING 	"OperatingMode"

uint32_t p3ServerConfig::getOperatingMode()
{
	std::string modestr = mGeneralConfig->getSetting(RS_CONFIG_OPERATING_STRING);
	uint32_t mode = RS_OPMODE_FULL;

	if (modestr == "FULL")
	{
		mode = RS_OPMODE_FULL;
	}
	else if (modestr == "NOTURTLE")
	{
		mode = RS_OPMODE_NOTURTLE;
	}
	else if (modestr == "GAMING")
	{
		mode = RS_OPMODE_GAMING;
	}
	else if (modestr == "MINIMAL_TRANSFER")
	{
		mode = RS_OPMODE_MINIMAL;
	}
	return mode;
}

bool p3ServerConfig::setOperatingMode(uint32_t opMode)
{
	std::string modestr = "FULL";
	switch(opMode)
	{
		case RS_OPMODE_FULL:
			modestr = "FULL";
		break;
		case RS_OPMODE_NOTURTLE:
			modestr = "NOTURTLE";

		break;
		case RS_OPMODE_GAMING:
			modestr = "GAMING";

		break;
		case RS_OPMODE_MINIMAL:
			modestr = "MINIMAL_TRANSFER";
		break;
	}
	mGeneralConfig->setSetting(RS_CONFIG_OPERATING_STRING, modestr);
	return switchToOperatingMode(opMode);
}

bool p3ServerConfig::switchToOperatingMode(uint32_t opMode)
{
	switch (opMode)
	{
		default:
		case RS_OPMODE_FULL:
			/* switch on all transfers */
			/* 100% bandwidth */
			/* switch on popups, enable hashing */
		break;
		case RS_OPMODE_NOTURTLE:
			/* switch on all transfers - except turtle, enable hashing */
			/* 100% bandwidth */
			/* switch on popups, enable hashing */

		break;
		case RS_OPMODE_GAMING:
			/* switch on all transfers */
			/* reduce bandwidth to 25% */
			/* switch off popups, enable hashing */
		break;
		case RS_OPMODE_MINIMAL:
			/* switch off all transfers */
			/* reduce bandwidth to 10%, but make sure there is enough for VoIP */
			/* switch on popups, enable hashing */
		break;
	}
	return true;
}


/* handle data rates.
 * Mutex must be handled at the lower levels: TODO */

int p3ServerConfig::SetDataRates( int downKb, int upKb ) /* in kbrates */
{
        //pqih -> setMaxRate(true, totalDownload);
        //pqih -> setMaxRate(false, totalUpload);

        //pqih -> save_config();
        return 1;
}


int p3ServerConfig::GetDataRates( float &inKb, float &outKb ) /* in kbrates */
{
        //pqih -> getCurrentRates(inKb, outKb);
        return 1;
}


