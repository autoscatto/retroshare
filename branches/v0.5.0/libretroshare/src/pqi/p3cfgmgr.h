/*
 * libretroshare/src/pqi: p3cfgmgr.h
 *
 * 3P/PQI network interface for RetroShare.
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



#ifndef P3_CONFIG_MGR_HEADER
#define P3_CONFIG_MGR_HEADER

#include <string>
#include <map>

#include "pqi/pqi_base.h"
#include "pqi/pqiindic.h"
#include "pqi/pqinetwork.h"
#include "util/rsthreads.h"
#include "pqi/pqibin.h"

/***** Configuration Management *****
 *
 * we need to store:
 * (1) Certificates.
 * (2) List of Friends / Net Configuration
 * (3) Stun List. / DHT peers.
 * (4) general config.
 *
 *
 * At top level we need:
 *
 * - type / filename / size / hash -
 * and the file signed...
 *
 *
 */

/**** THESE are STORED in CONFIGURATION FILES....
 * Cannot be changed
 *
 *********************/

const uint32_t CONFIG_TYPE_GENERAL 	    = 0x0001;
const uint32_t CONFIG_TYPE_PEERS 	    = 0x0002;
const uint32_t CONFIG_TYPE_FSERVER 	    = 0x0003;
const uint32_t CONFIG_TYPE_MSGS 	       = 0x0004;
const uint32_t CONFIG_TYPE_CACHE_OLDID  = 0x0005;
const uint32_t CONFIG_TYPE_AUTHGPG	 	 = 0x006;

const uint32_t CONFIG_TYPE_P3DISC	 	 = 0x00B;
const uint32_t CONFIG_TYPE_AUTHSSL	 	 = 0x00C;


/* new FileTransfer */
const uint32_t CONFIG_TYPE_FT_SHARED 	 = 0x0007;
const uint32_t CONFIG_TYPE_FT_EXTRA_LIST= 0x0008;
const uint32_t CONFIG_TYPE_FT_CONTROL 	 = 0x0009;
const uint32_t CONFIG_TYPE_FT_DWLQUEUE	 = 0x000A;

/* turtle router */
const uint32_t CONFIG_TYPE_TURTLE	 	 = 0x0020;

/* wish these ids where higher...
 * may move when switch to v0.5
 */
const uint32_t CONFIG_TYPE_RANK_LINK 	 = 0x0011;
const uint32_t CONFIG_TYPE_CHAT 	       = 0x0012;

/* standard services */
const uint32_t CONFIG_TYPE_QBLOG 	= 0x0101;
const uint32_t CONFIG_TYPE_FORUMS 	= 0x0102;
const uint32_t CONFIG_TYPE_CHANNELS 	= 0x0103;

/* CACHE ID Must be at the END so that other configurations
 * are loaded First (Cache Config --> Cache Loading)
 */
const uint32_t CONFIG_TYPE_CACHE 	= 0xff01;

class p3ConfigMgr;


/*!
 * Aim is that active objects in retroshare can dervie from this class
 * and implement their method of saving and loading their configuration
 */
class pqiConfig
{
	public:
	pqiConfig(uint32_t t);
virtual ~pqiConfig();

/**
 * loadHash This is the hash that will be compared to confirm saved configuration has not
 * been tampered with
 */
virtual bool	loadConfiguration(std::string &loadHash) = 0;


virtual bool	saveConfiguration() = 0;

/**
 * The type of configuration, see ids where this class is declared
 * @see p3cfgmgr.h
 */
uint32_t   Type();

/**
 *  The name of the configuration file
 */
std::string Filename();

/**
 * The hash computed for this configuration, can use this to compare to externally stored hash
 * for validation checking
 */
std::string Hash();

	protected:

/**
 * Checks if configuration has changed
 */
void	IndicateConfigChanged();
void	setHash(std::string h);

	RsMutex cfgMtx;

	private:

	/**
	 * This sets the name of the pqi configuation file
	 */
	void    setFilename(std::string name);

	/**
	 * @param an index for the Confind which contains list of configuarations that can be tracked
	 */
	bool    HasConfigChanged(uint16_t idx);

	Indicator ConfInd;

	uint32_t    type;
	std::string filename;
	std::string hash;

	friend class p3ConfigMgr;
	/* so it can access:
	 * setFilename() and HasConfigChanged()
	 */
};



/***********************************************************************************************/

/*!
 * MUTEX NOTE
 * None - because no-one calls any functions besides tick() when the system is running.
 * Takes care of updating, saving configurations within retroshare and also
 * adding new configurations
 */
class p3ConfigMgr
{
	public:
        p3ConfigMgr(std::string bdir, std::string fname, std::string signame);

        /**
         * checks and update all added configurations
         * @see rsserver
         */
        void	tick();

        /**
         * save all added configuation including configuration files
         */
        void	saveConfiguration();

        /**
         * save all loaded configurations
         */
        void	loadConfiguration();

        /**
         * @param file The name for new configuration
         * @param conf to the configuration to use
         */
        void	addConfiguration(std::string file, pqiConfig *conf);

		/* saves config, and disables further saving
		 * used for exiting the system
		 */
		void	completeConfiguration();

	private:

		/**
		 * checks if signature and configuration file's signature matches
		 * @return false if signature file does not match configuration file's signature
		 */
		bool getSignAttempt(std::string& metaConfigFname, std::string& metaSignFname, BinMemInterface* membio);

		/**
		 * takes current configuration which is stored in back-up file, and moves it to actual config file
		 * then stores data that was in actual config file into back-up, Corresponding signatures and handled
		 * simlarly in parallel
		 *@param fname the name of the first configuration file checked
		 *@param fname_backup the seconf file, backup, checked if first file does not exist or is corrupted
		 *@param sign file name in which signature is kept
		 *@param sign_backup  file name in which signature which correspond to backup config is kept
		 *@param configbio to write config to file
		 *@param signbio  to write signature config to file
		 */
		bool backedUpFileSave(const std::string& fname, const std::string& fname_backup,const std::string& sign,
				const std::string& sign_backup, BinMemInterface* configbio, BinMemInterface* signbio);

const std::string basedir;
const std::string metafname;
const std::string metasigfname;

	RsMutex cfgMtx; /* below is protected */

bool	mConfigSaveActive;
std::map<uint32_t, pqiConfig *> configs;
};

/***************************************************************************************************/

class p3Config: public pqiConfig
{
	public:

	p3Config(uint32_t t);

virtual bool	loadConfiguration(std::string &loadHash);
virtual bool	saveConfiguration();

	protected:

	/* Key Functions to be overloaded for Full Configuration */
virtual RsSerialiser *setupSerialiser() = 0;
virtual std::list<RsItem *> saveList(bool &cleanup) = 0;
virtual bool	loadList(std::list<RsItem *> load) = 0;
/**
 * callback for mutex unlocking
 * in derived classes (should only be needed if cleanup = false)
 */
virtual void    saveDone() { return; }

private:

/**
 * takes current configuration which is stored in back-up file, and moves it to actual config file
 * then stores data that was in actual config file into back-up, This is an rs specific solution
 * @param fname the name of the first configuration file checked
 * @param fname_backup the seconf file, backup, checked if first file does not exist or is corrupted
 */
bool backedUpFileSave(const std::string& fname, const std::string& fname_backup, std::list<RsItem* >& toSave,
		bool cleanup);

/*
 * for retrieving hash files
 */
bool getHashAttempt(const std::string& loadHash, std::string& hashstr, const std::string& fname, std::list<RsItem *>& load);


}; /* end of p3Config */


class p3GeneralConfig: public p3Config
{
	public:
	p3GeneralConfig();

// General Configuration System
std::string 	getSetting(std::string opt);
void 		setSetting(std::string opt, std::string val);

	protected:

	/* Key Functions to be overloaded for Full Configuration */
virtual RsSerialiser *setupSerialiser();
virtual std::list<RsItem *> saveList(bool &cleanup);
virtual bool	loadList(std::list<RsItem *> load);

	private:

	/* protected by pqiConfig mutex as well! */
std::map<std::string, std::string> settings;


};







#endif // P3_CONFIG_MGR_HEADER
