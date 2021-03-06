#ifndef RSDATASERVICE_H
#define RSDATASERVICE_H

/*
 * libretroshare/src/gxs: rsdataservice.h
 *
 * General Data service, interface for RetroShare.
 *
 * Copyright 2011-2012 by Evi-Parker Christopher
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

#include "gxs/rsgds.h"
#include "util/retrodb.h"


class RsDataService : public RsGeneralDataService
{
public:

    RsDataService(const std::string& serviceDir, const std::string& dbName, uint16_t serviceType, RsGxsSearchModule* mod = NULL);
    virtual ~RsDataService();

    /*!
     * Retrieves all msgs
     * @param reqIds requested msg ids (grpId,msgId), leave msg list empty to get all msgs for the grp
     * @param msg result of msg retrieval
     * @param cache whether to store results of this retrieval in memory for faster later retrieval
     * @return error code
     */
    int retrieveNxsMsgs(const GxsMsgReq& reqIds, GxsMsgResult& msg, bool cache, bool withMeta = false);

    /*!
     * Retrieves groups, if empty, retrieves all grps, if map is not empty
     * only retrieve entries, if entry cannot be found, it is removed from map
     * @param grp retrieved groups
     * @param withMeta this initialise the metaData member of the nxsgroups retrieved
     * @param cache whether to store retrieval in mem for faster later retrieval
     * @return error code
     */
    int retrieveNxsGrps(std::map<std::string, RsNxsGrp*>& grp, bool withMeta, bool cache);

    /*!
     * Retrieves meta data of all groups stored (most current versions only)
     * @param cache whether to store retrieval in mem for faster later retrieval
     * @return error code
     */
    int retrieveGxsGrpMetaData(std::map<RsGxsGroupId, RsGxsGrpMetaData*>& grp);

    /*!
     * Retrieves meta data of all groups stored (most current versions only)
     * @param grpIds grpIds for which to retrieve meta data
     * @param msgMeta meta data result as map of grpIds to array of metadata for that grpId
     * @param cache whether to store retrieval in mem for faster later retrieval
     * @return error code
     */
    int retrieveGxsMsgMetaData(const GxsMsgReq& reqIds, GxsMsgMetaResult& msgMeta);

    /*!
     * remove msgs in data store
     * @param grpId group Id of message to be removed
     * @param msgIds ids of messages to be removed
     * @return error code
     */
    int removeMsgs(const GxsMsgReq& msgIds);

    /*!
     * remove groups in data store listed in grpIds param
     * @param grpIds ids of groups to be removed
     * @return error code
     */
    int removeGroups(const std::vector<RsGxsGroupId>& grpIds);

    /*!
     * @return the cache size set for this RsGeneralDataService in bytes
     */
    uint32_t cacheSize() const;

    /*!
     * @param size size of cache to set in bytes
     */
    int setCacheSize(uint32_t size);

    /*!
     * Stores a list of signed messages into data store
     * @param msg map of message and decoded meta data information
     * @return error code
     */
    int storeMessage(std::map<RsNxsMsg*, RsGxsMsgMetaData*>& msg);

    /*!
     * Stores a list of groups in data store
     * @param grp map of group and decoded meta data
     * @return error code
     */
    int storeGroup(std::map<RsNxsGrp*, RsGxsGrpMetaData*>& grp);

    /*!
     * @param metaData The meta data item to update
     * @return error code
     */
    int updateMessageMetaData(MsgLocMetaData& metaData);

    /*!
     * @param metaData The meta data item to update
     * @return error code
     */
    int updateGroupMetaData(GrpLocMetaData& meta);


    /*!
     * Completely clear out data stored in
     * and returns this to a state
     * as it was when first constructed
     * @return error code
     */
    int resetDataStore();

private:

    /*!
     * Retrieves all the msg results from a cursor
     * @param c cursor to result set
     * @param msgs messages retrieved from cursor are stored here
     */
    void retrieveMessages(RetroCursor* c, std::vector<RsNxsMsg*>& msgs);

    /*!
     * Retrieves all the grp results from a cursor
     * @param c cursor to result set
     * @param grps groups retrieved from cursor are stored here
     */
    void retrieveGroups(RetroCursor* c, std::vector<RsNxsGrp*>& grps);

    /*!
     * Retrieves all the msg meta results from a cursor
     * @param c cursor to result set
     * @param metaSet message metadata retrieved from cursor are stored here
     */
    void retrieveMsgMeta(RetroCursor* c, std::vector<RsGxsMsgMetaData*>& msgMeta);

    /*!
     * extracts a msg meta item from a cursor at its
     * current position
     */
    RsGxsMsgMetaData* getMsgMeta(RetroCursor& c);

    /*!
     * extracts a grp meta item from a cursor at its
     * current position
     */
    RsGxsGrpMetaData* getGrpMeta(RetroCursor& c);

    /*!
     * extracts a msg item from a cursor at its
     * current position
     */
    RsNxsMsg* getMessage(RetroCursor& c);

    /*!
     * extracts a grp item from a cursor at its
     * current position
     */
    RsNxsGrp* getGroup(RetroCursor& c);

    /*!
     * Creates an sql database and its associated file
     * also creates the message and groups table
     */
    void initialise();

private:


    RetroDb* mDb;

    RsMutex mDbMutex;

    std::list<std::string> msgColumns;
    std::list<std::string> msgMetaColumns;

    std::list<std::string> grpColumns;
    std::list<std::string> grpMetaColumns;

    std::string mServiceDir, mDbName;
    uint16_t mServType;
};

#endif // RSDATASERVICE_H
