/*
 * libretroshare/src/ft: ftextralist.cc
 *
 * File Transfer for RetroShare.
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

#include "ft/ftextralist.h"
#include "serialiser/rsconfigitems.h"
#include "util/rsdir.h"
#include <stdio.h>

/******
 * #define DEBUG_ELIST	1
 *****/

ftExtraList::ftExtraList()
        :p3Config(CONFIG_TYPE_FT_EXTRA_LIST)
{
    return;
}


void ftExtraList::run()
{
    bool todo = false;
    time_t cleanup = 0;
    time_t now = 0;

    while (1)
    {
#ifdef  DEBUG_ELIST
        //std::cerr << "ftExtraList::run() Iteration";
        //std::cerr << std::endl;
#endif

        now = time(NULL);

        {
            RsStackMutex stack(extMutex);

            todo = (mToHash.size() > 0);
        }

        if (todo)
        {
            /* Hash a file */
            hashAFile();

#ifdef WIN32
            Sleep(1);
#else
            /* microsleep */
            usleep(10);
#endif
        }
        else
        {
            /* cleanup */
            if (cleanup < now)
            {
                cleanupOldFiles();
                cleanup = now + CLEANUP_PERIOD;
            }

            /* sleep */
#ifdef WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
        }
    }
}



void ftExtraList::hashAFile()
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::hashAFile()";
    std::cerr << std::endl;
#endif

    /* extract entry from the queue */
    FileDetails details;

    {
        RsStackMutex stack(extMutex);

        if (mToHash.size() == 0)
            return;

        details = mToHash.front();
        mToHash.pop_front();
    }

#ifdef  DEBUG_ELIST
    std::cerr << "Hashing: " << details.info.path;
    std::cerr << std::endl;
#endif

    /* hash it! */
    std::string name, hash;
    //uint64_t size;
    if (RsDirUtil::hashFile(details.info.path, details.info.fname,
                            details.info.hash, details.info.size))
    {
        RsStackMutex stack(extMutex);

        /* stick it in the available queue */
        mFiles[details.info.hash] = details;

        /* add to the path->hash map */
        mHashedList[details.info.path] = details.info.hash;

        IndicateConfigChanged();
    }
}

/***
 * If the File is alreay Hashed, then just add it in.
 **/

bool	ftExtraList::addExtraFile(std::string path, std::string hash,
                               uint64_t size, uint32_t period, uint32_t flags)
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::addExtraFile() path: " << path;
    std::cerr << " hash: " << hash;
    std::cerr << " size: " << size;
    std::cerr << " period: " << period;
    std::cerr << " flags: " << flags;

    std::cerr << std::endl;
#endif

    RsStackMutex stack(extMutex);

    FileDetails details;

    details.info.path = path;
    details.info.fname = RsDirUtil::getTopDir(path);
    details.info.hash = hash;
    details.info.size = size;
    details.info.age = time(NULL) + period; /* if time > this... cleanup */
    details.flags = flags;

    /* stick it in the available queue */
    mFiles[details.info.hash] = details;

    IndicateConfigChanged();

    return true;
}

bool ftExtraList::removeExtraFile(std::string hash, uint32_t flags)
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::removeExtraFile()";
    std::cerr << " hash: " << hash;
    std::cerr << " flags: " << flags;

    std::cerr << std::endl;
#endif

    RsStackMutex stack(extMutex);

    std::map<std::string, FileDetails>::iterator it;
    it = mFiles.find(hash);
    if (it == mFiles.end())
    {
        return false;
    }

    mFiles.erase(it);

    IndicateConfigChanged();

    return true;
}

bool ftExtraList::moveExtraFile(std::string fname, std::string hash, uint64_t size,
                                std::string destpath)
{
    RsStackMutex stack(extMutex);

    std::map<std::string, FileDetails>::iterator it;
    it = mFiles.find(hash);
    if (it == mFiles.end())
    {
        return false;
    }

    std::string path = destpath + '/' + fname;
    if (0 == rename(it->second.info.path.c_str(), path.c_str()))
    {
        /* rename */
        it->second.info.path = path;
        it->second.info.fname = fname;
        IndicateConfigChanged();
    }

    return true;
}



bool	ftExtraList::cleanupOldFiles()
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::cleanupOldFiles()";
    std::cerr << std::endl;
#endif

    RsStackMutex stack(extMutex);

    time_t now = time(NULL);

    std::list<std::string> toRemove;
    std::list<std::string>::iterator rit;

    std::map<std::string, FileDetails>::iterator it;
    for (it = mFiles.begin(); it != mFiles.end(); it++)
    {
        /* check timestamps */
        if (it->second.info.age < (unsigned) now)
        {
            toRemove.push_back(it->first);
        }
    }

    if (toRemove.size() > 0)
    {
        /* remove items */
        for (rit = toRemove.begin(); rit != toRemove.end(); rit++)
        {
            if (mFiles.end() != (it = mFiles.find(*rit)))
            {
                cleanupEntry(it->second.info.path, it->second.flags);
                mFiles.erase(it);
            }
        }
        IndicateConfigChanged();
    }
    return true;
}


bool	ftExtraList::cleanupEntry(std::string path, uint32_t flags)
{
    if (flags & RS_FILE_CONFIG_CLEANUP_DELETE)
    {
        /* Delete the file? - not yet! */
    }
    return true;
}

/***
 * Hash file, and add to the files,
 * file is removed after period.
 **/

bool 	ftExtraList::hashExtraFile(std::string path, uint32_t period, uint32_t flags)
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::hashExtraFile() path: " << path;
    std::cerr << " period: " << period;
    std::cerr << " flags: " << flags;

    std::cerr << std::endl;
#endif

    /* add into queue */
    RsStackMutex stack(extMutex);

    FileDetails details(path, period, flags);
    details.info.age = time(NULL) + period;
    mToHash.push_back(details);

    return true;
}

bool	ftExtraList::hashExtraFileDone(std::string path, FileInfo &info)
{
#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::hashExtraFileDone()";
    std::cerr << std::endl;
#endif

    std::string hash;
    {
        /* Find in the path->hash map */
        RsStackMutex stack(extMutex);

        std::map<std::string, std::string>::iterator it;
        if (mHashedList.end() == (it = mHashedList.find(path)))
        {
            return false;
        }
        hash = it->second;
    }
    return search(hash, 0, 0, info);
}

/***
 * Search Function - used by File Transfer
 *
 **/
bool    ftExtraList::search(std::string hash, uint64_t size, uint32_t hintflags, FileInfo &info) const
{

#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::search()";
    std::cerr << std::endl;
#endif

    /* find hash */
    std::map<std::string, FileDetails>::const_iterator fit;
    if (mFiles.end() == (fit = mFiles.find(hash)))
    {
        return false;
    }

    info = fit->second.info;
    return true;
}


/***
 * Configuration - store extra files.
 *
 **/

RsSerialiser *ftExtraList::setupSerialiser()
{
    RsSerialiser *rss = new RsSerialiser();

    /* add in the types we need! */
    rss->addSerialType(new RsFileConfigSerialiser());
    return rss;
}

std::list<RsItem *> ftExtraList::saveList(bool &cleanup)
{
    std::list<RsItem *> sList;

    cleanup = true;

    /* called after each item is added */

    /* create a list of fileitems with
     * age used to specify its timeout.
     */

#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::saveList()";
    std::cerr << std::endl;
#endif

    RsStackMutex stack(extMutex);


    std::map<std::string, FileDetails>::const_iterator it;
    for (it = mFiles.begin(); it != mFiles.end(); it++)
    {
        RsFileConfigItem *fi = new RsFileConfigItem();
        fi->file.path        = (it->second).info.path;
        fi->file.name        = (it->second).info.fname;
        fi->file.hash        = (it->second).info.hash;
        fi->file.filesize    = (it->second).info.size;
        fi->file.age         = (it->second).info.age;
        fi->flags            = (it->second).flags;

        sList.push_back(fi);
    }

    return sList;
}


bool    ftExtraList::loadList(std::list<RsItem *> load)
{
    /* for each item, check it exists ....
     * - remove any that are dead (or flag?)
     */

#ifdef  DEBUG_ELIST
    std::cerr << "ftExtraList::loadList()";
    std::cerr << std::endl;
#endif

    time_t ts = time(NULL);


    std::list<RsItem *>::iterator it;
    for (it = load.begin(); it != load.end(); it++)
    {

        RsFileConfigItem *fi = dynamic_cast<RsFileConfigItem *>(*it);
        if (!fi)
        {
            delete (*it);
            continue;
        }

        /* open file */
        FILE *fd = fopen(fi->file.path.c_str(), "rb");
        if (fd == NULL)
        {
            delete (*it);
            continue;
        }

        fclose(fd);

        if (ts > fi->file.age)
        {
            /* to old */
            cleanupEntry(fi->file.path, fi->flags);
            delete (*it);
            continue ;
        }

        /* add into system */
        FileDetails file;

        RsStackMutex stack(extMutex);

        FileDetails details;

        details.info.path = fi->file.path;
        details.info.fname = fi->file.name;
        details.info.hash = fi->file.hash;
        details.info.size = fi->file.filesize;
        details.info.age = fi->file.age; /* time that we remove it. */
        details.flags = fi->flags;

        /* stick it in the available queue */
        mFiles[details.info.hash] = details;
        delete (*it);

        /* short sleep */
#ifndef WINDOWS_SYS
        /********************************** WINDOWS/UNIX SPECIFIC PART ******************/
        usleep(1000); /* 1000 per second */

#else
        /********************************** WINDOWS/UNIX SPECIFIC PART ******************/
        Sleep(1);
#endif
        /********************************** WINDOWS/UNIX SPECIFIC PART ******************/

    }
    return true;
}

