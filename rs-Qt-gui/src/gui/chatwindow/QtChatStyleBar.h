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

#ifndef QTCHATSTYLEBAR_H
#define QTCHAtSTYLEBAR_H

#include <QtGui/QWidget>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QMutex>

class RetroStyleLabel;
class QString;


class QtChatStyleBar : public QWidget {
	Q_OBJECT
public:

	/**
	 * Constructor.
	 */
	QtChatStyleBar(QWidget * parent);

	/**
	 * Initialize the bar with the first & the last label.
	 */
	void init(RetroStyleLabel * firstLabel, RetroStyleLabel * endLabel);

	/**
	 * Adds a new button in the action bar associated to a unique text-based identifier.
	 *
	 * @param identifier : the text based identifier to associate with the button.
	 * @param normalPixmap : the image to be used for the button.
	 * @param pressedPixmap : the image to be used when the button is pressed.
	 * @param size : the size of the button.
	 * @param position : if specified, th e button is inserted at that position, else inserted at the end.
	 */
	void addLabel(const QString & identifier, const QPixmap & normalPixmap,
		const QPixmap & pressedPixmap, const QSize & size, bool isToggled = false, bool isText = false, int position = -1);

	/**
	 * Removes the button associated to the text-based identifier.
	 *
	 * @param identifier : the text based identifier of the button to remove.
	 */
	void removeLabel(const QString & identifier);

	/**
	 * Add a separator.
	 *
	 * @param position : if specified, the position where to insert the separator, else insert a the end.
	 */
	void addSeparator(const QString & pixmap, int position = -1);

	/**
	 * Removes a separator at a certain rank.
	 *
	 * @param ord : the rank of the separator to remove.
	 */
	void removeSeparator(int ord);

	/**
	 * Used to keep track of the added labels by string indentifiers.
	 */
	QMap<QString, RetroStyleLabel *> _labels;

private:

	/**
	 * The first RetroStyleLabel.
	 */
	RetroStyleLabel * _firstLabel;

	/**
	 * The "unremovable" end of the action bar.
	 */
	RetroStyleLabel * _endLabel;

	/**
	 * Used to keep track of the indexes of the labels in the layout and be abel to get them.
	 */
	QList<RetroStyleLabel *> _labelsIndexes;

	/**
	 * Used to keep track of the separators.
	 */
	QList<RetroStyleLabel *> _separators;

	QMutex _mutex;
};

#endif	//QTCHATSTYLEBAR_H
