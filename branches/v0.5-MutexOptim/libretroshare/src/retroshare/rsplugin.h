/*
 * "$Id: rsiface.h,v 1.9 2007-04-21 19:08:51 rmf24 Exp $"
 *
 * RetroShare C++ Interface.
 *
 * Copyright 2011-2011 by Cyril Soler
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

#pragma once

#include <time.h>
#include <stdint.h>
#include <string>
#include <vector>

class RsPluginHandler ;
extern RsPluginHandler *rsPlugins ;

class p3Service ;
class p3LinkMgr ;
class MainPage ;
class QIcon ;
class QString ;
class QWidget ;
class RsCacheService ;
class ftServer ;
class pqiService ;

// Used for the status of plugins.
//
#define PLUGIN_STATUS_NO_STATUS      0x0000
#define PLUGIN_STATUS_UNKNOWN_HASH   0x0001
#define PLUGIN_STATUS_DLOPEN_ERROR   0x0002
#define PLUGIN_STATUS_MISSING_SYMBOL 0x0003
#define PLUGIN_STATUS_NULL_PLUGIN    0x0004
#define PLUGIN_STATUS_LOADED         0x0005

class RsPlugin
{
	public:
		virtual RsCacheService *rs_cache_service() 	const	{ return NULL ; }
		virtual pqiService     *rs_pqi_service() 		const	{ return NULL ; }
		virtual uint16_t        rs_service_id() 	   const	{ return 0    ; }

		virtual MainPage       *qt_page()       		const	{ return NULL ; }
		virtual QWidget        *qt_config_panel()		const	{ return NULL ; }
		virtual QIcon          *qt_icon()       		const	{ return NULL ; }

		virtual std::string configurationFileName() const { return std::string() ; }
		virtual std::string getShortPluginDescription() const = 0 ;
		virtual std::string getPluginName() const = 0 ;
		virtual void getPluginVersion(int& major,int& minor,int& svn_rev) const = 0 ;
};

class RsPluginHandler
{
	public:
		// Returns the number of loaded plugins.
		//
		virtual int nbPlugins() const = 0 ;
		virtual RsPlugin *plugin(int i) = 0 ;
		virtual const std::vector<std::string>& getPluginDirectories() const = 0;
		virtual void getPluginStatus(int i,uint32_t& status,std::string& file_name,std::string& file_hash,std::string& error_string) const = 0 ;
		virtual void enablePlugin(const std::string& hash) = 0;
		virtual void disablePlugin(const std::string& hash) = 0;

		virtual void slowTickPlugins(time_t sec) = 0 ;

		virtual const std::string& getLocalCacheDir() const =0;
		virtual const std::string& getRemoteCacheDir() const =0;
		virtual ftServer *getFileServer() const = 0;
		virtual p3LinkMgr *getLinkMgr() const = 0;
};



