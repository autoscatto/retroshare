/*
 * libretroshare/src/services: p3idservice.h
 *
 * Identity interface for RetroShare.
 *
 * Copyright 2012-2012 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2.1 as published by the Free Software Foundation.
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

#ifndef P3_IDENTITY_SERVICE_HEADER
#define P3_IDENTITY_SERVICE_HEADER


#include "retroshare/rsidentity.h"	// External Interfaces.
#include "gxs/rsgenexchange.h"		// GXS service.
#include "gxs/rsgixs.h"			// Internal Interfaces.

#include <map>
#include <string>

/* 
 * Identity Service
 *
 */

// INTERNAL DATA TYPES. 
// Describes data stored in GroupServiceString.
class IdRepCumulScore
{
public:
	uint32_t count;
	uint32_t nullcount;
	double   sum;
	double   sumsq;
	
	// derived parameters:
};


class IdGroupServiceStrData
{
public:
	IdGroupServiceStrData() { pgpIdKnown = false; }
	bool pgpIdKnown;
	std::string pgpId;
	
	uint32_t ownScore;
	IdRepCumulScore opinion;
	IdRepCumulScore reputation;
	
};

#define ID_LOCAL_STATUS_FULL_CALC_FLAG	0x00010000
#define ID_LOCAL_STATUS_INC_CALC_FLAG	0x00020000



#define MAX_CACHE_SIZE	100 // Small for testing..
//#define MAX_CACHE_SIZE	10000 // More useful size

class RsGxsIdGroupItem;

class RsGxsIdCache
{
	public:
	RsGxsIdCache();
	RsGxsIdCache(const RsGxsIdGroupItem *item);

	RsGxsId id;
	std::string name;
	RsTlvSecurityKey pubkey;
	double reputation;
	time_t lastUsedTs;

};


class LruData
{
	public:
	RsGxsId key;
};

	

// Not sure exactly what should be inherited here?
// Chris - please correct as necessary.

class p3IdService: 
	public RsGxsIdExchange, 
	public RsIdentity
{
	public:
	p3IdService(RsGeneralDataService* gds, RsNetworkExchangeService* nes);

	virtual void service_tick(); // needed for background processing.


	/* General Interface is provided by RsIdentity / RsGxsIfaceImpl. */

	/* Data Specific Interface */

	// These are exposed via RsIdentity.
virtual bool getGroupData(const uint32_t &token, std::vector<RsGxsIdGroup> &groups);

	// These are local - and not exposed via RsIdentity.
virtual bool getMsgData(const uint32_t &token, std::vector<RsGxsIdOpinion> &opinions);
virtual bool createGroup(uint32_t& token, RsGxsIdGroup &group);
virtual bool createMsg(uint32_t& token, RsGxsIdOpinion &opinion);

	/**************** RsIdentity External Interface.
	 * Notes:
	 */

virtual bool  getNickname(const RsGxsId &id, std::string &nickname);
virtual bool  getIdDetails(const RsGxsId &id, RsIdentityDetails &details);
virtual bool  getOwnIds(std::list<RsGxsId> &ownIds);

        // 
virtual bool submitOpinion(uint32_t& token, RsIdOpinion &opinion);
virtual bool createIdentity(uint32_t& token, RsIdentityParameters &params);


	/**************** RsGixs Implementation 
	 * Notes:
	 *   Interface is only suggestion at the moment, will be changed as necessary.
	 *   Results should be cached / preloaded for maximum speed.
	 *
	 */
virtual bool haveKey(const RsGxsId &id);
virtual bool havePrivateKey(const RsGxsId &id);
virtual bool requestKey(const RsGxsId &id, const std::list<PeerId> &peers);
virtual int  getKey(const RsGxsId &id, RsTlvSecurityKey &key);
virtual int  getPrivateKey(const RsGxsId &id, RsTlvSecurityKey &key);  

	/**************** RsGixsReputation Implementation 
	 * Notes:
	 *   Again should be cached if possible.
	 */

        // get Reputation.
virtual bool getReputation(const RsGxsId &id, const GixsReputation &rep);


	protected:

    /** Notifications **/
    virtual void notifyChanges(std::vector<RsGxsNotify*>& changes);

	private:

/************************************************************************
 * This is the Cache for minimising calls to the DataStore.
 *
 */
	int  cache_tick();

	bool cache_is_loaded(const RsGxsId &id);
	bool cache_fetch(const RsGxsId &key, RsGxsIdCache &data);
	bool cache_store(const RsGxsIdGroupItem *item);

	bool cache_store(const RsGxsIdGroup &group);
	bool cache_resize();
	bool cache_discard_LRU(int count_to_clear);

	bool cache_request_load(const RsGxsId &id);
	bool cache_start_load();
	bool cache_check_loading();
	bool cache_load_for_token(uint32_t token);
	bool cache_check_consistency();

	/* MUTEX PROTECTED DATA (mIdMtx - maybe should use a 2nd?) */

	bool locked_cache_update_lrumap(const RsGxsId &key, time_t old_ts, time_t new_ts);

	std::map<RsGxsId, RsGxsIdCache> mCacheDataMap;
	std::multimap<time_t, LruData> mCacheLruMap;
	uint32_t mCacheDataCount;

	time_t mCacheLoad_LastCycle;
	int mCacheLoad_Status;
	std::list<RsGxsId> mCacheLoad_ToCache;
	std::list<uint32_t> mCacheLoad_Tokens;


/************************************************************************
 * Below is the background task for processing opinions => reputations 
 *
 */

virtual void generateDummyData();

std::string genRandomId();

	int	background_tick();
	bool background_checkTokenRequest();
	bool background_requestGroups();
	bool background_requestNewMessages();
	bool background_processNewMessages();
	bool background_FullCalcRequest();
	bool background_processFullCalc();
	
	bool background_cleanup();

	bool encodeIdGroupCache(std::string &str, const IdGroupServiceStrData &data);
	bool extractIdGroupCache(std::string &str, IdGroupServiceStrData &data);
	
	RsMutex mIdMtx;

	/***** below here is locked *****/
	bool mLastBgCheck;
	bool mBgProcessing;
	
	uint32_t mBgToken;
	uint32_t mBgPhase;
	
	std::map<std::string, RsGroupMetaData> mBgGroupMap;
	std::list<std::string> mBgFullCalcGroups;

/************************************************************************
 * Other Data that is protected by the Mutex.
 */

	std::vector<RsGxsGroupChange*> mGroupChange;
	std::vector<RsGxsMsgChange*> mMsgChange;

};

#endif // P3_IDENTITY_SERVICE_HEADER



