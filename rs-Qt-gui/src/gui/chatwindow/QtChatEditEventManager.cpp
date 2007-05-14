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

#include "QtChatEditEventManager.h"

//#include <util/Logger.h>

QtChatEditEventManager::QtChatEditEventManager(QObject * parent, QTextEdit * target) : QObject (parent) {
	_parent = parent;
	_target = target;
	_target->installEventFilter(this);
	_target->viewport()->installEventFilter(this);
}

bool QtChatEditEventManager::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::KeyPress) {
		if (keyPress(obj,event)) {
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

bool QtChatEditEventManager::keyPress(QObject *obj, QEvent *event) {
	QKeyEvent * e = static_cast<QKeyEvent *>(event);
	if ((e->key() == Qt::Key_Enter) || (e->key() == Qt::Key_Return)) {
		event->accept();
		enterPressed(e->modifiers());
		return true;
	}
	if ((e->key() == Qt::Key_Backspace)) {
		event->accept();
		deletePressed();
		return true;
	}
	if ((e->key() == Qt::Key_Tab)) {
		if ( (e->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
			event->accept();
			ctrlTabPressed();
			return true;
		}
	}
	return false;
}
