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

#include "QtChatEditWidget.h"

#include <QtGui/QtGui>

//#include <util/Logger.h>

QtChatEditWidget::QtChatEditWidget(QWidget *parent)
: QTextEdit(parent) {
	setFrameStyle(QFrame::NoFrame | QFrame::Plain);
	setAcceptDrops(true);
	setLineWrapMode(QTextEdit::WidgetWidth);
}

void QtChatEditWidget::dragEnterEvent(QDragEnterEvent *event) {
	if(event->mimeData()->hasFormat("text/uri-list")) {
		event->acceptProposedAction();
	}
}

void QtChatEditWidget::dragMoveEvent(QDragMoveEvent *event) {
	event->acceptProposedAction();
}

void QtChatEditWidget::dragLeaveEvent(QDragLeaveEvent * event) {
	event->accept();
}

