/****************************************************************
 *  RetroShare GUI is distributed under the following license:
 *
 *  Copyright (C) 2012 by Thunder
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

#ifndef RETROSHARE_FEEDREADER_GUI_INTERFACE_H
#define RETROSHARE_FEEDREADER_GUI_INTERFACE_H

#include <inttypes.h>
#include <string>
#include <list>

class RsFeedReader;
extern RsFeedReader *rsFeedReader;

enum RsFeedAddResult
{
	RS_FEED_ADD_RESULT_SUCCESS,
	RS_FEED_ADD_RESULT_FEED_NOT_FOUND,
	RS_FEED_ADD_RESULT_PARENT_NOT_FOUND,
	RS_FEED_ADD_RESULT_PARENT_IS_NO_FOLDER,
	RS_FEED_ADD_RESULT_FEED_IS_FOLDER,
	RS_FEED_ADD_RESULT_FEED_IS_NO_FOLDER
};

class FeedInfo
{
public:
	enum WorkState
	{
		WAITING,
		WAITING_TO_DOWNLOAD,
		DOWNLOADING,
		WAITING_TO_PROCESS,
		PROCESSING
	};

public:
	FeedInfo()
	{
		proxyPort = 0;
		updateInterval = 0;
		lastUpdate = 0;
		storageTime = 0;
		error = false;
		flag.folder = false;
		flag.infoFromFeed = false;
		flag.standardStorageTime = false;
		flag.standardUpdateInterval = false;
		flag.standardProxy = false;
		flag.authentication = false;
		flag.deactivated = false;
		flag.forum = false;
		flag.updateForumInfo = false;
		flag.embedImages = false;
		flag.saveCompletePage = false;
	}

	std::string  feedId;
	std::string  parentId;
	std::string  url;
	std::string  name;
	std::string  description;
	std::string  icon;
	std::string  user;
	std::string  password;
	std::string  proxyAddress;
	uint16_t     proxyPort;
	uint32_t     updateInterval;
	time_t       lastUpdate;
	uint32_t     storageTime;
	std::string  forumId;
	WorkState    workstate;
	bool         error;
	std::string  errorString;

	struct {
		bool folder : 1;
		bool infoFromFeed : 1;
		bool standardStorageTime : 1;
		bool standardUpdateInterval : 1;
		bool standardProxy : 1;
		bool authentication : 1;
		bool deactivated : 1;
		bool forum : 1;
		bool updateForumInfo : 1;
		bool embedImages : 1;
		bool saveCompletePage : 1;
	} flag;
};

class FeedMsgInfo
{
public:
	FeedMsgInfo()
	{
		pubDate = 0;
		flag.isnew = false;
		flag.read = false;
	}

	std::string msgId;
	std::string feedId;
	std::string title;
	std::string link;
	std::string author;
	std::string description;
	time_t      pubDate;

	struct {
		bool isnew : 1;
		bool read : 1;
	} flag;
};

class RsFeedReaderNotify
{
public:
	RsFeedReaderNotify() {}

	virtual void feedChanged(const std::string &/*feedId*/, int /*type*/) {}
	virtual void msgChanged(const std::string &/*feedId*/, const std::string &/*msgId*/, int /*type*/) {}
};

class RsFeedReader
{
public:
	RsFeedReader() {}
	virtual ~RsFeedReader() {}

	virtual void stop() = 0;
	virtual void setNotify(RsFeedReaderNotify *notify) = 0;

	virtual uint32_t getStandardStorageTime() = 0;
	virtual void     setStandardStorageTime(uint32_t storageTime) = 0;
	virtual uint32_t getStandardUpdateInterval() = 0;
	virtual void     setStandardUpdateInterval(uint32_t updateInterval) = 0;
	virtual bool     getStandardProxy(std::string &proxyAddress, uint16_t &proxyPort) = 0;
	virtual void     setStandardProxy(bool useProxy, const std::string &proxyAddress, uint16_t proxyPort) = 0;

	virtual RsFeedAddResult addFolder(const std::string parentId, const std::string &name, std::string &feedId) = 0;
	virtual RsFeedAddResult setFolder(const std::string &feedId, const std::string &name) = 0;
	virtual RsFeedAddResult addFeed(const FeedInfo &feedInfo, std::string &feedId) = 0;
	virtual RsFeedAddResult setFeed(const std::string &feedId, const FeedInfo &feedInfo) = 0;
	virtual bool            removeFeed(const std::string &feedId) = 0;
	virtual void            getFeedList(const std::string &parentId, std::list<FeedInfo> &feedInfos) = 0;
	virtual bool            getFeedInfo(const std::string &feedId, FeedInfo &feedInfo) = 0;
	virtual bool            getMsgInfo(const std::string &feedId, const std::string &msgId, FeedMsgInfo &msgInfo) = 0;
	virtual bool            removeMsg(const std::string &feedId, const std::string &msgId) = 0;
	virtual bool            removeMsgs(const std::string &feedId, const std::list<std::string> &msgIds) = 0;
	virtual bool            getMessageCount(const std::string &feedId, uint32_t *msgCount, uint32_t *newCount, uint32_t *unreadCount) = 0;
	virtual bool            getFeedMsgList(const std::string &feedId, std::list<FeedMsgInfo> &msgInfos) = 0;
	virtual bool            processFeed(const std::string &feedId) = 0;
	virtual bool            setMessageRead(const std::string &feedId, const std::string &msgId, bool read) = 0;

	/* get Ids */
//	virtual uint32_t getRankingsCount()			= 0;
//	virtual float   getMaxRank()				= 0;
//	virtual bool    getRankings(uint32_t first, uint32_t count, std::list<std::string> &rids) = 0;
//	virtual bool    getRankDetails(std::string rid, RsRankDetails &details) = 0;

	/* Add New Comment / Msg */
//	virtual std::string newRankMsg(std::wstring link, std::wstring title, std::wstring comment, int32_t score) = 0;
//	virtual bool updateComment(std::string rid, std::wstring comment, int32_t score) = 0;

//	virtual std::string anonRankMsg(std::string rid, std::wstring link, std::wstring title) = 0;
};

#endif
