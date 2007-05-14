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

#ifndef QTEMOTICONBUTTON_H
#define QTEMOTICONBUTTON_H

#include "QtEmoticon.h"

#include <QtGui/QToolButton>

class QWidget;
class QString;
class QIcon;


class QtEmoticonButton : public QToolButton {
	Q_OBJECT
public:

	QtEmoticonButton(QWidget * parent);

	void setEmoticon(QtEmoticon emoticon);

Q_SIGNALS:

	void buttonClicked(QtEmoticon emoticon);

private Q_SLOTS:

	void buttonClickedSlot();

private:

	QtEmoticon _emoticon;
};

#endif	//QTEMOTICONBUTTON_H
