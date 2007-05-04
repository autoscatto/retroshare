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

#ifndef WIDGETBACKGROUNDIMAGE_H
#define WIDGETBACKGROUNDIMAGE_H

#include <util/rsqtutildll.h>

#include <util/NonCopyable.h>

class QWidget;

/**
 * Replacement for QWidget::setBackgroundPixmap() that does not exist in Qt4 anymore.
 *
 * Draws a background image inside a QWidget.
 *
 * 
 */
class WidgetBackgroundImage : NonCopyable {
public:

	/**
	 * Sets a background image to a QWidget.
	 *
	 * @param widget QWidget that will get the background image
	 * @param imageFile background image filename
	 * @param resizeWidget true if widget should be resize to fit the image; false otherwise
	 */
	RSQTUTIL_API static void setBackgroundImage(QWidget * widget, const char * imageFile, bool resizeWidget);
};

#endif	//WIDGETBACKGROUNDIMAGE_H
