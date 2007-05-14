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

#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include "ui_ChatWidget.h"

#include "emoticons/QtEmoticon.h"


#include <QtGui/QtGui>


class QtChatEditActionBarWidget;
class QtChatEditWidget;
class QtChatHistoryWidget;

class QtChatAvatarFrame;

class RetroStyleLabel;
class EmoticonsWidget;
class QFont;
class QUrl;
class QTimerEvent;


class QtChatWidget : public QDialog
{
	Q_OBJECT
public:
    /** Default Constructor */
	QtChatWidget( QWidget *parent = 0, Qt::WFlags flags = 0);

      /** Destructor. */
	//~QtChatWidget();



     /** Overloaded QWidget.show */
     void show();

	/**
	 * Adds a message to the history and display it.
	 *
	 * @param senderName name of the contact who sends the message
	 * @param str the message
	 */
	void addToHistory(const QString & senderName, const QString & str);

	/**
	 * Displays a satus message.
	 *
	 * Behaves same as addToHistory but set the text color to the status message
	 * color and does not display a contact name.
	 */
	void addStatusMessage(const QString & statusMessage);



	/**
	 * Sets the Contact state.
	 *
	 * If not connected and the last status was connected,
	 * the ChatEditWidget will be disabled and a status message
	 * will be displayed saying the contact went offline.
	 *
	 * If connect and the last status was disconnected,
	 * the ChatEditWidget will be enabled and a status message
	 * will be displayed saying the contact when online.
	 **/


public Q_SLOTS:

	/**
	 * QtChatEditWidget text has changed
	 */
	void chatEditTextChanged();

	void enterPressed(Qt::KeyboardModifiers modifier = Qt::NoModifier);

	void sendButtonClicked();

	void deletePressed();

	void changeFont();

	void chooseEmoticon();

	void emoticonSelected(QtEmoticon emoticon);





	virtual void setVisible(bool visible);



Q_SIGNALS:


private Q_SLOTS:

	void avatarFrameButtonClicked();

	void changeFontColor();

	void boldClickedSlot();

	void italicClickedSlot();

	void underlineClickedSlot();

	//void contactChangedSlot(QString contactId);

private:

	typedef QHash<QString, QString> UserColorHash;
	
	QString getUserColor(const QString & nickName) const {
		return _userColorHash[nickName];
	}
	
	virtual void timerEvent(QTimerEvent * event);

	void sendMessage();

	//void updateAvatarFrame();

	void updateUserAvatar();

	bool hasUserColor(const QString & nickname) const;

	/**
	 * @brief stop the timer associated to _notTypingTimerId
	 */
	void stopNotTypingTimer();

	/**
	 * @brief stop the timer associated to _stoppedTypingTimerId
	 */
	void stopStoppedTypingTimer();

	void addAvatarFrame();

	void removeAvatarFrame();

	QString getNewBackgroundColor() const;



	UserColorHash _userColorHash;

	QWidget * _widget;

	QString _contactId;

	int _sessionId;

	int _stoppedTypingTimerId;

	int _notTypingTimerId;

	bool _isTyping;

	bool _isAvatarFrameOpened;

	QString _nickName;

	mutable QColor _lastBackGroundColor;

	mutable QMutex _mutex;

	QtChatEditActionBarWidget * _editActionBar;

	QtChatEditWidget * _chatEdit;

	QtChatHistoryWidget * _chatHistory;

	QtChatAvatarFrame * _avatarFrame;

	EmoticonsWidget * _emoticonsWidget;

	Ui::ChatWidget _ui;


	// font style settings
	QFont _currentFont;
	QColor _currentColor;
	bool _bold;
	bool _italic;
	bool _underline;
	////
};

#endif //CHATWIDGET_H
