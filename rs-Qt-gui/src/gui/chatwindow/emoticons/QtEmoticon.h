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

#ifndef QTEMOTICON_H
#define QTEMOTICON_H

#include <QtCore/QStringList>
#include <QtCore/QString>
#include <QtGui/QPixmap>


class QtEmoticon {
public:

	QtEmoticon();

	QtEmoticon(const QtEmoticon & source);

	~QtEmoticon();

	void setText(const QStringList & text) { _text = text; }

	void setPath(const QString & path) { _path = path; }

	void setPixmap(const QPixmap & pixmap) { _pixmap = pixmap; }

	void setButtonPixmap(const QPixmap & pixmap) { _buttonPixmap = pixmap; }

	void setRegExp(const QString & regExp);

	QString getRegExp() const { return _regExp; }

	QStringList getText() const { return _text;}

	QString getDefaultText() const { return _text[0]; }

	QPixmap getPixmap() const { return _pixmap; }

	QPixmap getButtonPixmap() const { return _buttonPixmap; }

	QString getPath() const { return _path; }

	QString getHtmlRegExp() const;

	QString getHtml() const;

	bool isNull() const;

	QtEmoticon & operator=(const QtEmoticon & source);

private:

	QString _path;

	QStringList _text;

	QString _regExp;

	QPixmap _pixmap;

	QPixmap _buttonPixmap;
};

#endif	//QTEMOTICON_H
