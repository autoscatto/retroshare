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

#include "QtEmoticonButton.h"

#include <QtGui/QtGui>

//#include "util/SafeConnect.h"

QtEmoticonButton::QtEmoticonButton(QWidget * parent)
	: QToolButton(parent) {

	setAutoRaise(true);
	connect(this, SIGNAL(clicked()), SLOT(buttonClickedSlot()));
}

void QtEmoticonButton::buttonClickedSlot() {
	buttonClicked(_emoticon);
}

void QtEmoticonButton::setEmoticon(QtEmoticon emoticon) {
	_emoticon = emoticon;
	setIcon(QIcon(emoticon.getButtonPixmap()));
	setIconSize(emoticon.getButtonPixmap().size());
	if (!emoticon.getText().isEmpty()) {
		setToolTip(emoticon.getText()[0]);
	}
}
