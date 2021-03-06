/***************************************************************************
 *   Copyright (C) 2009                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include <QStringList>
#include <QRegExp>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>
#include <QDesktopServices>
#include <QMessageBox>
#include <QIcon>
#include <QObject>

#include "RetroShareLink.h"
#include "MainWindow.h"
#include "ForumsDialog.h"
#include "ChannelFeed.h"
#include "SearchDialog.h"
#include "msgs/MessageComposer.h"
#include "util/misc.h"
#include "common/PeerDefs.h"
#include "common/RsCollectionFile.h"
#include "gui/connect/ConfCertDialog.h"

#include <retroshare/rsfiles.h>
#include <retroshare/rspeers.h>
#include <retroshare/rsforums.h>
#include <retroshare/rschannels.h>

//#define DEBUG_RSLINK 1

#define HOST_FILE       "file"
#define HOST_PERSON     "person"
#define HOST_FORUM      "forum"
#define HOST_CHANNEL    "channel"
#define HOST_MESSAGE    "message"
#define HOST_REGEXP     "file|person|forum|channel|search|message"

#define FILE_NAME       "name"
#define FILE_SIZE       "size"
#define FILE_HASH       "hash"

#define PERSON_NAME     "name"
#define PERSON_HASH     "hash"

#define FORUM_NAME      "name"
#define FORUM_ID        "id"
#define FORUM_MSGID     "msgid"

#define CHANNEL_NAME    "name"
#define CHANNEL_ID      "id"
#define CHANNEL_MSGID   "msgid"

#define MESSAGE_ID      "id"
#define MESSAGE_SUBJECT "subject"

#define HOST_SEARCH     "search"
#define SEARCH_KEYWORDS "keywords"

RetroShareLink::RetroShareLink(const QUrl& url)
{
    fromUrl(url);
}

RetroShareLink::RetroShareLink(const QString& url)
{
    fromString(url);
}

void RetroShareLink::fromString(const QString& url)
{
    clear();

    // parse
#ifdef DEBUG_RSLINK
    std::cerr << "got new RS link \"" << url.toStdString() << "\"" << std::endl ;
#endif

    if ((url.startsWith(QString(RSLINK_SCHEME) + "://" + QString(HOST_FILE)) && url.count("|") == 3) ||
        (url.startsWith(QString(RSLINK_SCHEME) + "://" + QString(HOST_PERSON)) && url.count("|") == 2)) {
        /* Old link, we try it */
        QStringList list = url.split ("|");

        if (list.size() >= 1) {
            if (list.size() == 4 && list[0] == QString(RSLINK_SCHEME) + "://" + QString(HOST_FILE)) {
                bool ok ;

                _type = TYPE_FILE;
                _name = list[1] ;
                _size = list[2].toULongLong(&ok) ;
                _hash = list[3].left(40) ;	// normally not necessary, but it's a security.

                if (ok) {
#ifdef DEBUG_RSLINK
                    std::cerr << "New RetroShareLink forged:" << std::endl ;
                    std::cerr << "  name = \"" << _name.toStdString() << "\"" << std::endl ;
                    std::cerr << "  hash = \"" << _hash.toStdString() << "\"" << std::endl ;
                    std::cerr << "  size = " << _size << std::endl ;
#endif
                    check();
                    return;
                }
            } else if (list.size() == 3 && list[0] == QString(RSLINK_SCHEME) + "://" + QString(HOST_PERSON)) {
                _type = TYPE_PERSON;
                _name = list[1] ;
                _hash = list[2].left(40) ;	// normally not necessary, but it's a security.
                _size = 0;
                check();
                return;
            }

            // bad link
        }
    }

    /* Now try QUrl */
    fromUrl(QUrl::fromEncoded(url.toUtf8().constData()));
}

void RetroShareLink::fromUrl(const QUrl& url)
{
    clear();

    // parse
#ifdef DEBUG_RSLINK
    std::cerr << "got new RS link \"" << url.toString().toStdString() << "\"" << std::endl ;
#endif

    if (url.scheme() != RSLINK_SCHEME) {
        /* No RetroShare-Link */
        return;
    }

    if (url.host() == HOST_FILE) {
        bool ok ;

        _type = TYPE_FILE;
        _name = url.queryItemValue(FILE_NAME);
        _size = url.queryItemValue(FILE_SIZE).toULongLong(&ok);
        _hash = url.queryItemValue(FILE_HASH).left(40);	// normally not necessary, but it's a security.

        if (ok) {
#ifdef DEBUG_RSLINK
            std::cerr << "New RetroShareLink forged:" << std::endl ;
            std::cerr << "  name = \"" << _name.toStdString() << "\"" << std::endl ;
            std::cerr << "  hash = \"" << _hash.toStdString() << "\"" << std::endl ;
            std::cerr << "  size = " << _size << std::endl ;
#endif
            check();
            return;
        }
    }

    if (url.host() == HOST_PERSON) {
        _type = TYPE_PERSON;
        _name = url.queryItemValue(PERSON_NAME);
        _hash = url.queryItemValue(PERSON_HASH).left(40);	// normally not necessary, but it's a security.
        check();
        return;
    }

    if (url.host() == HOST_FORUM) {
        _type = TYPE_FORUM;
        _name = url.queryItemValue(FORUM_NAME);
        _hash = url.queryItemValue(FORUM_ID);
        _msgId = url.queryItemValue(FORUM_MSGID);
        check();
        return;
    }

    if (url.host() == HOST_CHANNEL) {
        _type = TYPE_CHANNEL;
        _name = url.queryItemValue(CHANNEL_NAME);
        _hash = url.queryItemValue(CHANNEL_ID);
        _msgId = url.queryItemValue(CHANNEL_MSGID);
        check();
        return;
    }

    if (url.host() == HOST_SEARCH) {
        _type = TYPE_SEARCH;
        _name = url.queryItemValue(SEARCH_KEYWORDS);
        check();
        return;
    }

    if (url.host() == HOST_MESSAGE) {
        _type = TYPE_MESSAGE;
        std::string id = url.queryItemValue(MESSAGE_ID).toStdString();
        createMessage(id, url.queryItemValue(MESSAGE_SUBJECT));
        return;
    }

    // bad link

#ifdef DEBUG_RSLINK
    std::cerr << "Wrongly formed RS link. Can't process." << std::endl ;
#endif
    clear();
}

RetroShareLink::RetroShareLink()
{
    clear();
}

bool RetroShareLink::createFile(const QString& name, uint64_t size, const QString& hash)
{
    clear();

    _name = name;
    _size = size;
    _hash = hash;

    _type = TYPE_FILE;

    check();

    return valid();
}

bool RetroShareLink::createPerson(const std::string& id)
{
    clear();

    RsPeerDetails detail;
    if (rsPeers->getPeerDetails(id, detail) == false) {
        std::cerr << "RetroShareLink::createPerson() Couldn't find peer id " << id << std::endl;
        return false;
    }

    _hash = QString::fromStdString(id);
    _name = QString::fromUtf8(detail.name.c_str());

    _type = TYPE_PERSON;

    check();

    return valid();
}

bool RetroShareLink::createForum(const std::string& id, const std::string& msgId)
{
	clear();

	if (!id.empty()) {
		_hash = QString::fromStdString(id);
		_msgId = QString::fromStdString(msgId);

		_type = TYPE_FORUM;

		if (msgId.empty()) {
			ForumInfo fi;
			if (rsForums->getForumInfo(id, fi)) {
				_name = QString::fromStdWString(fi.forumName);
			}
		} else {
			ForumMsgInfo mi;
			if (rsForums->getForumMessage(id, msgId, mi)) {
				_name = QString::fromStdWString(mi.title);
			}
		}
	}

	check();

	return valid();
}

bool RetroShareLink::createChannel(const std::string& id, const std::string& msgId)
{
	clear();

	if (!id.empty()) {
		_hash = QString::fromStdString(id);
		_msgId = QString::fromStdString(msgId);

		_type = TYPE_CHANNEL;

		if (msgId.empty()) {
			ChannelInfo ci;
			if (rsChannels->getChannelInfo(id, ci)) {
				_name = QString::fromStdWString(ci.channelName);
			}
		} else {
			ChannelMsgInfo mi;
			if (rsChannels->getChannelMessage(id, msgId, mi)) {
				_name = QString::fromStdWString(mi.subject);
			}
		}
	}

	check();

	return valid();
}

bool RetroShareLink::createSearch(const QString& keywords)
{
    clear();

    _name = keywords;

    _type = TYPE_SEARCH;

    check();

    return valid();
}

bool RetroShareLink::createMessage(const std::string& peerId, const QString& subject)
{
	clear();

	_hash = QString::fromStdString(peerId);
	PeerDefs::rsidFromId(peerId, &_name);
	_subject = subject;

	_type = TYPE_MESSAGE;

	check();

	return valid();
}

void RetroShareLink::clear()
{
    _valid = false;
    _type = TYPE_UNKNOWN;
    _hash = "" ;
    _size = 0 ;
    _name = "" ;
}

void RetroShareLink::check()
{
	_valid = true;

	switch (_type) {
	case TYPE_UNKNOWN:
		_valid = false;
		break;
	case TYPE_FILE:
		if(_size > (((uint64_t)1)<<40))	// 1TB. Who has such large files?
			_valid = false;

		if(!checkName(_name))
			_valid = false;

		if(!checkHash(_hash))
			_valid = false;
		break;
	case TYPE_PERSON:
		if(_size != 0)
			_valid = false;

		if(_name.isEmpty())
			_valid = false;

		if(_hash.isEmpty())
			_valid = false;
		break;
	case TYPE_FORUM:
		if(_size != 0)
			_valid = false;

		if(_name.isEmpty())
			_valid = false;

		if(_hash.isEmpty())
			_valid = false;
		break;
	case TYPE_CHANNEL:
		if(_size != 0)
			_valid = false;

		if(_name.isEmpty())
			_valid = false;

		if(_hash.isEmpty())
			_valid = false;
		break;
	case TYPE_SEARCH:
		if(_size != 0)
			_valid = false;

		if(_name.isEmpty())
			_valid = false;

		if(!_hash.isEmpty())
			_valid = false;
		break;
	case TYPE_MESSAGE:
		if(_size != 0)
			_valid = false;

		if(_hash.isEmpty())
			_valid = false;
		break;
	}

	if (!_valid) {
		clear();
	}
}

QString RetroShareLink::title() const
{
	if (!valid()) {
		return "";
	}

	switch (_type) {
	case TYPE_UNKNOWN:
		break;
	case TYPE_FILE:
		return QString("%1 (%2)").arg(hash()).arg(misc::friendlyUnit(size()));
	case TYPE_PERSON:
		return PeerDefs::rsidFromId(hash().toStdString());
	case TYPE_FORUM:
	case TYPE_CHANNEL:
	case TYPE_SEARCH:
		break;
	case TYPE_MESSAGE:
		return PeerDefs::rsidFromId(hash().toStdString());
	}

	return "";
}

static QString encodeItem(QString item)
{
	return item.replace(" ", "%20");
}

QString RetroShareLink::toString() const
{
    switch (_type) {
    case TYPE_UNKNOWN:
        break;
    case TYPE_FILE:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_FILE);
            url.addQueryItem(FILE_NAME, encodeItem(_name));
            url.addQueryItem(FILE_SIZE, QString::number(_size));
            url.addQueryItem(FILE_HASH, _hash);

            return url.toString();
        }
    case TYPE_PERSON:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_PERSON);
            url.addQueryItem(PERSON_NAME, encodeItem(_name));
            url.addQueryItem(PERSON_HASH, _hash);

            return url.toString();
        }
    case TYPE_FORUM:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_FORUM);
            url.addQueryItem(FORUM_NAME, encodeItem(_name));
            url.addQueryItem(FORUM_ID, _hash);
            if (!_msgId.isEmpty()) {
                url.addQueryItem(FORUM_MSGID, _msgId);
            }

            return url.toString();
        }
    case TYPE_CHANNEL:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_CHANNEL);
            url.addQueryItem(CHANNEL_NAME, encodeItem(_name));
            url.addQueryItem(CHANNEL_ID, _hash);
            if (!_msgId.isEmpty()) {
                url.addQueryItem(CHANNEL_MSGID, _msgId);
            }

            return url.toString();
        }
    case TYPE_SEARCH:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_SEARCH);
            url.addQueryItem(SEARCH_KEYWORDS, encodeItem(_name));

            return url.toString();
        }
    case TYPE_MESSAGE:
        {
            QUrl url;
            url.setScheme(RSLINK_SCHEME);
            url.setHost(HOST_MESSAGE);
            url.addQueryItem(MESSAGE_ID, _hash);
            if (_subject.isEmpty() == false) {
               url.addQueryItem(MESSAGE_SUBJECT, encodeItem(_subject));
            }

            return url.toString();
        }
    }

    return "";
}

QString RetroShareLink::niceName() const
{
    if (type() == TYPE_PERSON) {
        return PeerDefs::rsid(name().toUtf8().constData(), hash().toStdString());
    }

    return name();
}

QString RetroShareLink::toHtml() const
{
	QString html = "<a href=\"" + toString() + "\"";

	QString linkTitle = title();
	if (!linkTitle.isEmpty()) {
		html += " title=\"" + linkTitle + "\"";
	}
	html += ">" + niceName() + "</a>" ;

	return html;
}

QString RetroShareLink::toHtmlFull() const
{
	return QString("<a href=\"") + toString() + "\">" + toString() + "</a>" ;
}

QString RetroShareLink::toHtmlSize() const
{
	QString size = QString("(%1)").arg(misc::friendlyUnit(_size));
	if (type() == TYPE_FILE && RsCollectionFile::isCollectionFile(name())) {
		FileInfo finfo;
		if (rsFiles->FileDetails(hash().toStdString(), RS_FILE_HINTS_EXTRA | RS_FILE_HINTS_LOCAL, finfo)) {
			RsCollectionFile collection;
			if (collection.load(QString::fromUtf8(finfo.path.c_str()), false)) {
				size += QString(" [%1]").arg(misc::friendlyUnit(collection.size()));
			}
		}
	}
	QString link = QString("<a href=\"%1\">%2</a> <font color=\"blue\">%3</font>").arg(toString()).arg(name()).arg(size);
	return link;
}

bool RetroShareLink::checkName(const QString& name)
{
    if(name == "")
        return false ;

    for(int i=0;i<name.length();++i)
    {
        QChar::Category cat( name[i].category() ) ;

        if(	cat == QChar::Separator_Line
                || cat == QChar::Other_NotAssigned
                )
        {
#ifdef DEBUG_RSLINK
            std::cerr <<"Unwanted category " << cat << " at place " << i << " in string \"" << name.toStdString() << "\"" << std::endl ;
#endif
            return false ;
        }
    }

    return true ;
}

QUrl RetroShareLink::toUrl() const
{
    return QUrl::fromEncoded(toString().toUtf8().constData());
}

bool RetroShareLink::checkHash(const QString& hash)
{
    if(hash.length() != 40)
        return false ;

    QByteArray qb(hash.toAscii()) ;

    for(int i=0;i<qb.length();++i)
    {
        unsigned char b(qb[i]) ;

        if(!((b>47 && b<58) || (b>96 && b<103)))
            return false ;
    }

    return true ;
}

static void processList(QStringList &list, const QString &textSingular, const QString &textPlural, QString &result)
{
	if (list.size() == 0) {
		return;
	}
	if (list.size() == 1) {
		result += "" + textSingular + ":";
	} else {
		result += "" + textPlural + ":";
	}
	result += "<p style='margin-left: 5px; margin-top: 0px'>";
	QStringList::iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
		if (it != list.begin()) {
			result += ", ";
		}
		result += *it;
	}
	result += "</p>";
}

/*static*/ int RetroShareLink::process(QList<RetroShareLink> &linksIn, uint flag /*= RSLINK_PROCESS_NOTIFY_ALL*/)
{
	QList<RetroShareLink>::iterator linkIt;

	/* filter dublicate links */
	QList<RetroShareLink> links;
	for (linkIt = linksIn.begin(); linkIt != linksIn.end(); linkIt++) {
		if (links.contains(*linkIt)) {
			continue;
		}

		links.append(*linkIt);
	}

	if (flag & RSLINK_PROCESS_NOTIFY_ASK) {
		/* ask for some types of link */
		QStringList fileAdd;
		QStringList personAdd;

		for (linkIt = links.begin(); linkIt != links.end(); linkIt++) {
			RetroShareLink &link = *linkIt;

			if (link.valid() == false) {
				continue;
			}

			switch (link.type()) {
			case TYPE_UNKNOWN:
			case TYPE_FORUM:
			case TYPE_CHANNEL:
			case TYPE_SEARCH:
			case TYPE_MESSAGE:
				// no need to ask
				break;

			case TYPE_FILE:
				fileAdd.append(link.name());
				break;

			case TYPE_PERSON:
				personAdd.append(PeerDefs::rsid(link.name().toUtf8().constData(), link.hash().toStdString()));
				break;
			}
		}

		QString content;
		if (fileAdd.size()) {
			processList(fileAdd, QObject::tr("Add file"), QObject::tr("Add files"), content);
		}

		if (personAdd.size()) {
			processList(personAdd, QObject::tr("Add friend"), QObject::tr("Add friends"), content);
		}

		if (content.isEmpty() == false) {
			QString question = "<html><body>";
			if (links.size() == 1) {
				question += QObject::tr("Do you want to process the link ?");
			} else {
				question += QObject::tr("Do you want to process %1 links ?").arg(links.size());
			}
			question += "<br><br>" + content + "</body></html>";

			QMessageBox mb(QObject::tr("Confirmation"), question, QMessageBox::Question, QMessageBox::Yes,QMessageBox::No, 0);
			mb.setWindowIcon(QIcon(QString::fromUtf8(":/images/rstray3.png")));
			if (mb.exec() == QMessageBox::No) {
				return 0;
			}
		}
	}

	int countInvalid = 0;
	int countUnknown = 0;
	bool needNotifySuccess = false;

	// file
	QStringList fileAdded;
	QStringList fileExist;

	// person
	QStringList personAdded;
	QStringList personExist;
	QStringList personFailed;
	QStringList personNotFound;

	// forum
	QStringList forumFound;
	QStringList forumMsgFound;
	QStringList forumUnknown;
	QStringList forumMsgUnknown;

	// forum
	QStringList channelFound;
	QStringList channelMsgFound;
	QStringList channelUnknown;
	QStringList channelMsgUnknown;

	// search
	QStringList searchStarted;

	// message
	QStringList messageStarted;
	QStringList messageReceipientNotAccepted;
	QStringList messageReceipientUnknown;

	// summary
	QList<QStringList*> processedList;
	QList<QStringList*> errorList;

	processedList << &fileAdded << &personAdded << &forumFound << &channelFound << &searchStarted << &messageStarted;
	errorList << &fileExist << &personExist << &personFailed << &personNotFound << &forumUnknown << &forumMsgUnknown << &channelUnknown << &channelMsgUnknown << &messageReceipientNotAccepted << &messageReceipientUnknown;
	// not needed: forumFound, channelFound, messageStarted

	for (linkIt = links.begin(); linkIt != links.end(); linkIt++) {
		RetroShareLink &link = *linkIt;

		if (link.valid() == false) {
			std::cerr << " RetroShareLink::process invalid request" << std::endl;
			countInvalid++;
			continue;
		}

		switch (link.type()) {
		case TYPE_UNKNOWN:
			countUnknown++;
			break;

		case TYPE_FILE:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process FileRequest : fileName : " << link.name().toUtf8().constData() << ". fileHash : " << link.hash().toStdString() << ". fileSize : " << link.size() << std::endl;
#endif

				needNotifySuccess = true;

				// Get a list of available direct sources, in case the file is browsable only.
				std::list<std::string> srcIds;
				FileInfo finfo ;
				rsFiles->FileDetails(link.hash().toStdString(), RS_FILE_HINTS_REMOTE, finfo) ;

				for(std::list<TransferInfo>::const_iterator it(finfo.peers.begin());it!=finfo.peers.end();++it)
				{
#ifdef DEBUG_RSLINK
					std::cerr << "  adding peerid " << (*it).peerId << std::endl ;
#endif
					srcIds.push_back((*it).peerId) ;
				}

				if (rsFiles->FileRequest(link.name().toUtf8().constData(), link.hash().toStdString(), link.size(), "", RS_FILE_HINTS_NETWORK_WIDE, srcIds)) {
					fileAdded.append(link.name());
				} else {
					fileExist.append(link.name());
				}
				break;
			}

		case TYPE_PERSON:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process FriendRequest : name : " << link.name().toStdString() << ". id : " << link.hash().toStdString() << std::endl;
#endif

				needNotifySuccess = true;

				RsPeerDetails detail;
				if (rsPeers->getPeerDetails(link.hash().toStdString(), detail)) {
					if (detail.gpg_id == rsPeers->getGPGOwnId()) {
						// it's me, do nothing
						break;
					}

					if (detail.accept_connection) {
						// peer connection is already accepted
						personExist.append(PeerDefs::rsid(detail));
						break;
					}

					if (rsPeers->addFriend("", link.hash().toStdString())) {
						ConfCertDialog::loadAll();
						personAdded.append(PeerDefs::rsid(detail));
						break;
					}

					personFailed.append(PeerDefs::rsid(link.name().toUtf8().constData(), link.hash().toStdString()));
					break;
				}

				personNotFound.append(PeerDefs::rsid(link.name().toUtf8().constData(), link.hash().toStdString()));
				break;
			}

		case TYPE_FORUM:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process ForumRequest : name : " << link.name().toStdString() << ". id : " << link.hash().toStdString() << ". msgId : " << link.msgId().toStdString() << std::endl;
#endif

				ForumInfo fi;
				if (!rsForums->getForumInfo(link.id().toStdString(), fi)) {
					if (link.msgId().isEmpty()) {
						forumUnknown.append(link.name());
					} else {
						forumMsgUnknown.append(link.name());
					}
					break;
				}

				ForumMsgInfo msg;
				if (!link.msgId().isEmpty()) {
					if (!rsForums->getForumMessage(fi.forumId, link.msgId().toStdString(), msg)) {
						forumMsgUnknown.append(link.name());
						break;
					}
				}

				MainWindow::showWindow(MainWindow::Forums);
				ForumsDialog *forumsDialog = dynamic_cast<ForumsDialog*>(MainWindow::getPage(MainWindow::Forums));
				if (!forumsDialog) {
					return false;
				}

				if (forumsDialog->navigate(fi.forumId, msg.msgId)) {
					if (link.msgId().isEmpty()) {
						forumFound.append(link.name());
					} else {
						forumMsgFound.append(link.name());
					}
				} else {
					if (link.msgId().isEmpty()) {
						forumUnknown.append(link.name());
					} else {
						forumMsgUnknown.append(link.name());
					}
				}
				break;
			}

		case TYPE_CHANNEL:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process ChannelRequest : name : " << link.name().toStdString() << ". id : " << link.hash().toStdString() << ". msgId : " << link.msgId().toStdString() << std::endl;
#endif

				ChannelInfo ci;
				if (!rsChannels->getChannelInfo(link.id().toStdString(), ci)) {
					if (link.msgId().isEmpty()) {
						channelUnknown.append(link.name());
					} else {
						channelMsgUnknown.append(link.name());
					}
					break;
				}

				ChannelMsgInfo msg;
				if (!link.msgId().isEmpty()) {
					if (!rsChannels->getChannelMessage(ci.channelId, link.msgId().toStdString(), msg)) {
						channelMsgUnknown.append(link.name());
						break;
					}
				}

				MainWindow::showWindow(MainWindow::Channels);
				ChannelFeed *channelFeed = dynamic_cast<ChannelFeed*>(MainWindow::getPage(MainWindow::Channels));
				if (!channelFeed) {
					return false;
				}

				if (channelFeed->navigate(ci.channelId, msg.msgId)) {
					if (link.msgId().isEmpty()) {
						channelFound.append(link.name());
					} else {
						channelMsgFound.append(link.name());
					}
				} else {
					if (link.msgId().isEmpty()) {
						channelUnknown.append(link.name());
					} else {
						channelMsgUnknown.append(link.name());
					}
				}
				break;
			}

		case TYPE_SEARCH:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process SearchRequest : string : " << link.name().toStdString() << std::endl;
#endif

				MainWindow::showWindow(MainWindow::Search);
				SearchDialog *searchDialog = dynamic_cast<SearchDialog*>(MainWindow::getPage(MainWindow::Search));
				if (!searchDialog) {
					break;
				}

				searchDialog->searchKeywords(link.name());
				searchStarted.append(link.name());
				break;
			}

		case TYPE_MESSAGE:
			{
#ifdef DEBUG_RSLINK
				std::cerr << " RetroShareLink::process MessageRequest : id : " << link.hash().toStdString() << ", subject : " << link.name().toStdString() << std::endl;
#endif
				RsPeerDetails detail;
				if (rsPeers->getPeerDetails(link.hash().toStdString(), detail)) {
					if (detail.accept_connection || detail.id == rsPeers->getOwnId() || detail.id == rsPeers->getGPGOwnId()) {
						MessageComposer *msg = MessageComposer::newMsg();
						msg->addRecipient(MessageComposer::TO, detail.id, false);
						if (link.subject().isEmpty() == false) {
							msg->insertTitleText(link.subject());
						}
						msg->show();
						messageStarted.append(PeerDefs::nameWithLocation(detail));
					} else {
						messageReceipientNotAccepted.append(PeerDefs::nameWithLocation(detail));
					}
				} else {
					messageReceipientUnknown.append(PeerDefs::rsidFromId(link.hash().toStdString()));
				}

				break;
			}

		default:
			std::cerr << " RetroShareLink::process unknown type: " << link.type() << std::endl;
			countUnknown++;
		}
	}

	int countProcessed = 0;
	int countError = 0;

	QList<QStringList*>::iterator listIt;
	for (listIt = processedList.begin(); listIt != processedList.end(); ++listIt) {
		countProcessed += (*listIt)->size();
	}
	for (listIt = errorList.begin(); listIt != errorList.end(); ++listIt) {
		countError += (*listIt)->size();
	}

	// success notify needed ?
	if (needNotifySuccess == false) {
		flag &= ~RSLINK_PROCESS_NOTIFY_SUCCESS;
	}
	// error notify needed ?
	if (countError == 0) {
		flag &= ~RSLINK_PROCESS_NOTIFY_ERROR;
	}

	QString result;

	if (flag & (RSLINK_PROCESS_NOTIFY_SUCCESS | RSLINK_PROCESS_NOTIFY_ERROR)) {
		result += (links.count() == 1 ? QObject::tr("%1 of %2 RetroShare link processed.") : QObject::tr("%1 of %2 RetroShare links processed.")).arg(countProcessed).arg(links.count()) + "<br><br>";
	}

	// file
	if (flag & RSLINK_PROCESS_NOTIFY_SUCCESS) {
		if (fileAdded.size()) {
			processList(fileAdded, QObject::tr("File added"), QObject::tr("Files added"), result);
		}
	}
	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (fileExist.size()) {
			processList(fileExist, QObject::tr("File exist"), QObject::tr("Files exist"), result);
		}
	}

	// person
	if (flag & RSLINK_PROCESS_NOTIFY_SUCCESS) {
		if (personAdded.size()) {
			processList(personAdded, QObject::tr("Friend added"), QObject::tr("Friends added"), result);
		}
	}
	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (personExist.size()) {
			processList(personExist, QObject::tr("Friend exist"), QObject::tr("Friends exist"), result);
		}
		if (personFailed.size()) {
			processList(personFailed, QObject::tr("Friend not added"), QObject::tr("Friends not added"), result);
		}
		if (personNotFound.size()) {
			processList(personNotFound, QObject::tr("Friend not found"), QObject::tr("Friends not found"), result);
		}
	}

	// forum
	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (forumUnknown.size()) {
			processList(forumUnknown, QObject::tr("Forum not found"), QObject::tr("Forums not found"), result);
		}
		if (forumMsgUnknown.size()) {
			processList(forumMsgUnknown, QObject::tr("Forum message not found"), QObject::tr("Forum messages not found"), result);
		}
	}

	// channel
	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (channelUnknown.size()) {
			processList(channelUnknown, QObject::tr("Channel not found"), QObject::tr("Channels not found"), result);
		}
		if (channelMsgUnknown.size()) {
			processList(channelMsgUnknown, QObject::tr("Channel message not found"), QObject::tr("Channel messages not found"), result);
		}
	}

	// message
	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (messageReceipientNotAccepted.size()) {
			processList(messageReceipientNotAccepted, QObject::tr("Receipient not accepted"), QObject::tr("Receipients not accepted"), result);
		}
		if (messageReceipientUnknown.size()) {
			processList(messageReceipientUnknown, QObject::tr("Unkown receipient"), QObject::tr("Unkown receipients"), result);
		}
	}

	if (flag & RSLINK_PROCESS_NOTIFY_ERROR) {
		if (countUnknown) {
			result += QString("<br>%1: %2").arg(QObject::tr("Malformed links")).arg(countUnknown);
		}
		if (countInvalid) {
			result += QString("<br>%1: %2").arg(QObject::tr("Invalid links")).arg(countInvalid);
		}
	}

	if (result.isEmpty() == false) {
		QMessageBox mb(QObject::tr("Result"), "<html><body>" + result + "</body></html>", QMessageBox::Information, QMessageBox::Ok, 0, 0);
		mb.setWindowIcon(QIcon(QString::fromUtf8(":/images/rstray3.png")));
		mb.exec();
	}

	return 0;
}

/*static*/ int RetroShareLink::process(QStringList &urls, RetroShareLink::enumType type /*= RetroShareLink::TYPE_UNKNOWN*/, uint flag /*= RSLINK_PROCESS_NOTIFY_ALL*/)
{
	QList<RetroShareLink> links;

	for (QStringList::iterator it = urls.begin(); it != urls.end(); it++) {
		RetroShareLink link(*it);
		if (link.valid() && (type == RetroShareLink::TYPE_UNKNOWN || link.type() == type)) {
			links.append(link);
		}
	}

	return process(links, flag);
}

void RSLinkClipboard::copyLinks(const QList<RetroShareLink>& links)
{
    QString res ;
    for (int i = 0; i < links.size(); ++i)
        res += links[i].toString() + "\n" ;

    QApplication::clipboard()->setText(res) ;
}

void RSLinkClipboard::pasteLinks(QList<RetroShareLink> &links)
{
    return parseClipboard(links);
}

void RSLinkClipboard::parseClipboard(QList<RetroShareLink> &links)
{
    // parse clipboard for links.
    //
    links.clear();
    QString text = QApplication::clipboard()->text() ;

    std::cerr << "Parsing clipboard:" << text.toStdString() << std::endl ;

    QRegExp rx(QString("retroshare://(%1)[^\r\n]+").arg(HOST_REGEXP));

    int pos = 0;

    while((pos = rx.indexIn(text, pos)) != -1)
    {
        QString url(text.mid(pos, rx.matchedLength()));
        RetroShareLink link(url);

        if(link.valid())
        {
            // check that the link is not already in the list:
            bool already = false ;
            for (int i = 0; i <links.size(); ++i)
                if(links[i] == link)
                {
                    already = true ;
                    break ;
                }

            if(!already)
            {
                links.push_back(link) ;
#ifdef DEBUG_RSLINK
                std::cerr << "captured link: " << link.toString().toStdString() << std::endl ;
#endif
            }
        }
#ifdef DEBUG_RSLINK
        else
            std::cerr << "invalid link" << std::endl ;
#endif

        pos += rx.matchedLength();
    }
}

QString RSLinkClipboard::toString()
{
    QList<RetroShareLink> links;
    parseClipboard(links);

    QString res ;
    for(int i = 0; i < links.size(); ++i)
        res += links[i].toString() + "\n" ;

    return res ;
}

QString RSLinkClipboard::toHtml()
{
    QList<RetroShareLink> links;
    parseClipboard(links);

    QString res ;
    for(int i = 0; i < links.size(); ++i)
        res += links[i].toHtml() + "<br>" ;

    return res ;
}

QString RSLinkClipboard::toHtmlFull()
{
    QList<RetroShareLink> links;
    parseClipboard(links);

    QString res ;
    for(int i = 0; i < links.size(); ++i)
        res += links[i].toHtmlFull() + "<br>" ;

    return res ;
}

bool RSLinkClipboard::empty(RetroShareLink::enumType type /*= RetroShareLink::TYPE_UNKNOWN*/)
{
    QList<RetroShareLink> links;
    parseClipboard(links);

    if (type == RetroShareLink::TYPE_UNKNOWN) {
        return links.empty();
    }

    for (QList<RetroShareLink>::iterator link = links.begin(); link != links.end(); link++) {
        if (link->type() == type) {
            return false;
        }
    }

    return true;
}

/*static*/ int RSLinkClipboard::process(RetroShareLink::enumType type /*= RetroShareLink::TYPE_UNKNOWN*/, uint flag /*= RSLINK_PROCESS_NOTIFY_ALL*/)
{
    QList<RetroShareLink> links;
    pasteLinks(links);

	QList<RetroShareLink> linksToProcess;
	for (int i = 0; i < links.size(); i++) {
        if (links[i].valid() && (type == RetroShareLink::TYPE_UNKNOWN || links[i].type() == type)) {
            linksToProcess.append(links[i]);
        }
    }

	if (linksToProcess.size() == 0) {
		return 0;
	}

	return RetroShareLink::process(linksToProcess, flag);
}
