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

#ifndef OWQTCHATEDITEVENTMANAGER_H
#define OWQTCHATEDITEVENTMANAGER_H

#include <QtGui/QtGui>


class QtChatEditEventManager : public QObject {
	Q_OBJECT
public:

	QtChatEditEventManager (QObject * parent = 0, QTextEdit * target = 0);

Q_SIGNALS:

	void enterPressed(Qt::KeyboardModifiers modifier);

	void ctrlTabPressed();

	void deletePressed();

protected:

	bool eventFilter(QObject *obj, QEvent *event);

	bool keyPress(QObject *obj, QEvent *event);

	QTextEdit * _target;

	QObject * _parent;
};

#endif //OWQTCHATEDITEVENTMANAGER_H
