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

#include "QtChatEditActionBarWidget.h"

#include <util/RetroStyleLabel.h>

//#include <util/Logger.h>

#include <QtCore/QString>

QtChatEditActionBarWidget::QtChatEditActionBarWidget(QWidget * parent) :
	QtChatStyleBar(parent) {

	setMaximumSize(QSize(10000, 65));
	setMinimumSize(QSize(16, 65));

	// configure the first label
	_emoticonsLabel = new RetroStyleLabel(this);
	_emoticonsLabel->setPixmaps(
		QPixmap(":/images/chat/chat_emoticon_button.png"),
		QPixmap(),
		QPixmap(),
		QPixmap(":/images/chat/chat_emoticon_button_on.png"),
		QPixmap(),
		QPixmap()
	);
	_emoticonsLabel->setMaximumSize(QSize(46,65));
	_emoticonsLabel->setMinimumSize(QSize(46,65));
	connect(_emoticonsLabel, SIGNAL(clicked()), SIGNAL(emoticonsLabelClicked()));
	////

	// configure end label
	_endLabel = new RetroStyleLabel(this);
	_endLabel->setPixmaps(
		QPixmap(),
		QPixmap(":/images/chat/bar_end.png"),
		QPixmap(":/images/chat/bar_fill.png"),
		QPixmap(),
		QPixmap(":/images/chat/bar_end.png"),
		QPixmap(":/images/chat/bar_fill.png")
	);
	_endLabel->setMinimumSize(QSize(16, 65));
	////
	init(_emoticonsLabel, _endLabel);

	/*
	// add font label
	addLabel(QString("fontLabel"),
		QPixmap(":/images/chat/font_police.png"),
		QPixmap(":/images/chat/font_police_on.png"),
		QSize(36, 65)
	);
	connect(_labels["fontLabel"], SIGNAL(clicked()), SIGNAL(fontLabelClicked()));
	////
	*/

	/*
	// add font color label
	addLabel(QString("fontColorLabel"),
		QPixmap(":/images/chat/font_color.png"),
		QPixmap(":/images/chat/font_color_on.png"),
		QSize(36, 65)
	);
	connect(_labels["fontColorLabel"], SIGNAL(clicked()), SIGNAL(fontColorLabelClicked()));
	////
	*/

	// add bold label
	addLabel(QString("boldLabel"),
		QPixmap(":/images/chat/font_bold.png"),
		QPixmap(":/images/chat/font_bold_on.png"),
		QSize(36, 65),
		true
	);
	connect(_labels["boldLabel"], SIGNAL(clicked()), SIGNAL(boldLabelClicked()));
	////

	// add italic label
	addLabel(QString("italicLabel"),
		QPixmap(":/images/chat/font_italic.png"),
		QPixmap(":/images/chat/font_italic_on.png"),
		QSize(36, 65),
		true
	);
	connect(_labels["italicLabel"], SIGNAL(clicked()), SIGNAL(italicLabelClicked()));
	////

	// add underline label
	addLabel(QString("underlineLabel"),
		QPixmap(":/images/chat/font_underline.png"),
		QPixmap(":/images/chat/font_underline_on.png"),
		QSize(36, 65),
		true
	);
	connect(_labels["underlineLabel"], SIGNAL(clicked()), SIGNAL(underlineLabelClicked()));
	////
}
