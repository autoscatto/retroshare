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

#ifndef QTCHATHISTORYWIDGET_H
#define QTCHATHISTORYWIDGET_H

#include <QtGui/QTextBrowser>

class QPoint;
class QAction;
class QWidget;


class QtChatHistoryWidget : public QTextBrowser {
	Q_OBJECT
public:

	QtChatHistoryWidget(QWidget * parent);

public Q_SLOTS:

	void showContextMenuSlot(const QPoint & pos);

	//void saveHistoryAsHtmlSlot();

private Q_SLOTS:

	/**
	 * An URL has been clicked we must open a web browser
	 *
	 * @param link the clicked url
	 */
	//void urlClicked(const QUrl & link);

private:

	void makeActions();

	QAction * _saveAsAction;
};

#endif	//QTCHATHISTORYWIDGET_H
