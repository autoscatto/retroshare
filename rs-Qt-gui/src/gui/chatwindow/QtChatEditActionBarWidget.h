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

#ifndef QTCHATEDITACTIONBARWIDGET_H
#define QTCHATEDITACTIONBARWIDGET_H

#include "QtChatStyleBar.h"

class QtChatWidget;
class RetroStyleLabel;
class QString;


class QtChatEditActionBarWidget : public QtChatStyleBar {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtChatEditActionBarWidget(QWidget * parent);

Q_SIGNALS:

	/**
	 * Emoticon label has been clicked.
	 */
	void emoticonsLabelClicked();

	/**
	 * Font label has been clicked.
	 */
	void fontLabelClicked();

	/**
	 * Font color label has been clicked.
	 */
	void fontColorLabelClicked();

	/**
	 * Bold label has been clicked.
	 */
	void boldLabelClicked();

	/**
	 * Italic label has been clicked.
	 */
	void italicLabelClicked();

	/**
	 * Underline label has been clicked.
	 */
	void underlineLabelClicked();

private:

	RetroStyleLabel * _emoticonsLabel;

	RetroStyleLabel * _endLabel;
};

#endif	//QTCHATEDITACTIONBARWIDGET_H
