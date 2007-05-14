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

#include "QtChatUtils.h"

#include "emoticons/QtEmoticonsManager.h"

#include <QtGui/QtGui>
#include <QtXml/QtXml>

//TODO: remove this from here
static const QString CHAT_USER_FORGROUND_COLOR = "#000000";

const QString QtChatUtils::replaceUrls(const QString & str, const QString & htmlstr) {
	int beginPos = 0;
	QString tmp = htmlstr;
	int endPos;
	int repCount = 0;

	QStringList urls;
	QStringList reps;
	while (true) {

		beginPos = str.indexOf(QRegExp("(http://|https://|ftp://|^www.)", Qt::CaseInsensitive), beginPos);
		if (beginPos == -1) {
			break;
		}

		for (endPos = beginPos; endPos < str.size(); endPos++) {
			if ((str[endPos] == ' ') || (str[endPos] == '\r') || (str[endPos] == '\n')) {
				break;
			}
		}
		QString url = str.mid(beginPos, endPos - beginPos);
		urls << url;
		QString r = QString("$$_$$*_RETROSTRUTILS_|KB|%1").arg(repCount);
		reps << r;
		repCount++;
		tmp.replace(url, r);
		beginPos = endPos;
	}
	for (int i = 0; i < reps.size(); i++) {
		QString url = QString("<a href='" + urls[i]+ "'>" + urls[i] + "</a>");
		tmp.replace(reps[i], url);
	}
	return tmp;
}

const QString QtChatUtils::insertFontTag(QFont font, const QString & message) {
	QString result = "<font color=\"%1\"><font face=\"%2\">";
	result = result.arg(CHAT_USER_FORGROUND_COLOR);
	result = result.arg(font.defaultFamily());

	if (font.bold()) {
		result += "<b>";
	}

	if (font.italic()) {
		result += "<i>";
	}

	if (font.underline()) {
		result += "<u>";
	}

	result += message;

	if (font.underline()) {
		result += "</u>";
	}

	if (font.italic()) {
		result += "</i>";
	}

	if (font.bold()) {
		result += "</b>";
	}
	result += "</font></font>";
	return result;
}

const QString QtChatUtils::text2Emoticon(const QString & htmlstr, const QString & protocol) {
	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();
	return emoticonsManager->text2Emoticon(htmlstr, protocol);
}

const QString QtChatUtils::emoticons2Text(const QString & htmlstr, const QString & protocol) {
	QtEmoticonsManager * emoticonsManager = QtEmoticonsManager::getInstance();
	return emoticonsManager->emoticons2Text(htmlstr, protocol);
}





const QString QtChatUtils::getHeader(const QString & bgColor, const QString & textColor, const QString & message) {
	QString header = QString(
		"<table border=\"0\" width=\"100%\" cellspacing=\"0\""
		"cellpadding=\"3\"><tr>"
		"<td bgcolor=\"%1\">"
		"<span style=\" font-weight:600; color:%2; background-color:%1;\"> %3 </span>"
		 "</td>"
		"<td bgcolor=\"%4\" align=right>"
		"<span style=\" font-weight:600; color:%5; background-color:%1;\"> %6 </span>"
		"</td>"
		"</span>"
		"</tr></table>").
		arg(bgColor).
		arg(textColor).
		arg(message).
		arg(bgColor).
		arg(textColor).
		arg(QTime::currentTime().toString());
	return header;
}
