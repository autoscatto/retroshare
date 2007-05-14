/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,2007 crypton
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

#ifndef QTEMOTICONSMANAGER_H
#define QTEMOTICONSMANAGER_H

#include "QtEmoticon.h"

#include <QtCore/QHash>

class QDomNode;
class QString;

/**
 *
 * @ingroup presentation
 * @author Mathieu Stute
 */
class QtEmoticonsManager {
public:

	typedef QList<QtEmoticon> QtEmoticonList;

	static QtEmoticonsManager * getInstance();

	QtEmoticonsManager();

	QtEmoticon getEmoticon(const QString & text, const QString & protocol);

	void loadFromFile(const QString & filename);

	QString text2Emoticon(const QString & text, const QString & protocol);

	QString emoticons2Text(const QString & text, const QString & protocol);

	QtEmoticonList getQtEmoticonList(const QString & protocol);

	int getProtocolCount();

	int getEmoticonCount(const QString & protocol);

private:

	void readProtocol(const QDomNode & node);

	void readIcon(const QDomNode & node, const QString & protocol);

	QString encode(const QString & text) const;

	typedef QHash<QString, QtEmoticonList> ProtocolList;

	ProtocolList _protocolList;

	static QtEmoticonsManager * _instance;
};

#endif	//OWQTEMOTICONSMANAGER_H
