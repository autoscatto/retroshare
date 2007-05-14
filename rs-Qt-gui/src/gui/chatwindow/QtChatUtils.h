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

#ifndef QTCHATUTILS_H
#define QTCHATUTILS_H

//#include <imwrapper/EnumIMProtocol.h>

class QFont;
class QString;


class QtChatUtils {
public:

	
	/**
	 * Gets a HTML table to display a message and the current time.
	 *
	 * @param bgColor background color.
	 * @param textColor text color.
	 * @param message the message.
	 * @return the html table.
	 */
	static const QString getHeader(const QString & bgColor, const QString & textColor, const QString & message);

private:

	/**
	 * Replaces urls by href html code.
	 *
	 * @param str the message in plain text.
	 * @param htmlstr the message in html.
	 * @return the changed message.
	 */
	static const QString replaceUrls(const QString & str, const QString & htmlstr);

	/**
	 * Inserts a font tag based on font.
	 *
	 * @param font the font to use.
	 * @param message the message to change.
	 * @return the changed message.
	 */
	static const QString insertFontTag(QFont font, const QString & message);

	/**
	 * @see QtEmoticonsManager::text2Emoticon.
	 */
	static const QString text2Emoticon(const QString & htmlstr, const QString & protocol);

	/**
	 * @see QtEmoticonsManager::emoticon2Text.
	 */
	static const QString emoticons2Text(const QString & htmlstr, const QString & protocol);
};

#endif	//QTCHATUTILS_H
