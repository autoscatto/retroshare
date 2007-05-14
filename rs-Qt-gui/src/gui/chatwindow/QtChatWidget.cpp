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

#include "QtChatWidget.h"
#include "QtChatEditEventManager.h"
#include "QtChatEditActionBarWidget.h"
#include "QtChatEditWidget.h"
#include "QtChatHistoryWidget.h"
#include "QtChatUtils.h"
#include "QtChatAvatarFrame.h"

#include "emoticons/QtEmoticonsWidget.h"



#include <QtGui/QtGui>

#include <string>

static const int CHAT_NOT_TYPING_DELAY = 1000;
static const int CHAT_STOPPED_TYPING_DELAY = 1000;
static const QString CHAT_USER_BACKGOUND_COLOR = "#F0EFFF";
static const QString CHAT_USER_FOREGROUND_COLOR = "#000000";

QtChatWidget::QtChatWidget(QWidget *parent, Qt::WFlags flags)
:QDialog(parent, flags)
 {

	//Default nickname for testing purpose
	_nickName = "RetroShare";
	_stoppedTypingTimerId = -1;
	_notTypingTimerId = -1;
	_isTyping = false;

	_lastBackGroundColor = QColor("#D0FFE6");
	_isAvatarFrameOpened = true;

	_bold = false;
	_italic = false;
	_underline = false;



	//setup ui
	_ui.setupUi(this);
	QPixmap pix = _ui.sendButton->icon().pixmap(50,50);
	QPainter painter(&pix);
	painter.setPen(Qt::white);
	painter.drawText(pix.rect(),Qt::AlignCenter,tr("send"));
	painter.end();
	_ui.sendButton->setIcon(QIcon(pix));
	////

	//creates sub widgets
	_emoticonsWidget = new EmoticonsWidget(this);
	_editActionBar = new QtChatEditActionBarWidget(this);
	_chatEdit = new QtChatEditWidget(this);
	_chatHistory = new QtChatHistoryWidget(this);
	_avatarFrame = new QtChatAvatarFrame(this);
	////

	//create QtChatEditEventManager
	QtChatEditEventManager * cwm = new QtChatEditEventManager(this, _chatEdit);
	////

	//add sub widgets
	_ui.editActionStackedWidget->addWidget(_editActionBar);
	_ui.editActionStackedWidget->setCurrentWidget(_editActionBar);
	_ui.editStackedWidget->addWidget(_chatEdit);
	_ui.editStackedWidget->setCurrentWidget(_chatEdit);
	_ui.historyStackedWidget->addWidget(_chatHistory);
	_ui.historyStackedWidget->setCurrentWidget(_chatHistory);
	////

	//signal connection
	connect(cwm, SIGNAL(enterPressed(Qt::KeyboardModifiers)), SLOT(enterPressed(Qt::KeyboardModifiers)));
	connect(cwm, SIGNAL(deletePressed()), SLOT(deletePressed()));
	connect(_ui.sendButton, SIGNAL(clicked()), SLOT(sendButtonClicked()));
	connect(_editActionBar, SIGNAL(fontLabelClicked()), SLOT(changeFont()));
	connect(_editActionBar, SIGNAL(emoticonsLabelClicked()), SLOT(chooseEmoticon()));
	connect(_editActionBar, SIGNAL(fontColorLabelClicked()), SLOT(changeFontColor()));
	connect(_editActionBar, SIGNAL(boldLabelClicked()), SLOT(boldClickedSlot()));
	connect(_editActionBar, SIGNAL(italicLabelClicked()), SLOT(italicClickedSlot()));
	connect(_editActionBar, SIGNAL(underlineLabelClicked()), SLOT(underlineClickedSlot()));
	connect(_emoticonsWidget, SIGNAL(emoticonClicked(QtEmoticon)), SLOT(emoticonSelected(QtEmoticon)));
	connect(_emoticonsWidget, SIGNAL(closed()), _chatEdit, SLOT(setFocus()));
	connect(_chatEdit, SIGNAL(textChanged()), SLOT(chatEditTextChanged()));

	connect(_ui.avatarFrameButton, SIGNAL(clicked()), SLOT(avatarFrameButtonClicked()));


	addAvatarFrame();

}

//QtChatWidget::~QtChatWidget() {
//	close();
//}

void QtChatWidget::changeFont() {
	bool ok;
	QFont font = QFontDialog::getFont(&ok, _chatEdit->currentFont(), this);
	if (ok) {
		_currentFont = font;
		_chatEdit->setCurrentFont(font);
	}
	_chatEdit->setFocus();
}

void QtChatWidget::changeFontColor() {
	bool ok;
	QRgb color = QColorDialog::getRgba(_chatEdit->textColor().rgba(), &ok, this);
	if (ok) {
		_currentColor = QColor(color);
		_chatEdit->setTextColor(_currentColor);
	}
	_chatEdit->setFocus();
}

void QtChatWidget::boldClickedSlot() {
	_bold = !_bold;
	if (_bold) {
		_chatEdit->setFontWeight(QFont::Bold);
	} else {
		_chatEdit->setFontWeight(QFont::Normal);
	}
}

void QtChatWidget::italicClickedSlot() {
	_italic = !_italic;
	_chatEdit->setFontItalic(_italic);
}

void QtChatWidget::underlineClickedSlot() {
	_underline = !_underline;
	_chatEdit->setFontUnderline(_underline);
}

void QtChatWidget::chooseEmoticon() {
	QPoint p = _ui.editActionStackedWidget->pos();
	p.setY(p.y() + _ui.editActionStackedWidget->rect().bottom());
	_emoticonsWidget->move(mapToGlobal(p));
	_emoticonsWidget->setWindowOpacity(0.95);
	_emoticonsWidget->show();
}

void QtChatWidget::setVisible(bool visible) {
	QWidget::setVisible(visible);
	if (visible) {
		_chatEdit->setFocus();
	}
}




void QtChatWidget::sendButtonClicked() {
	enterPressed();
}

bool QtChatWidget::hasUserColor(const QString & nickName) const {
	return (_userColorHash.find(nickName) != _userColorHash.end());
}

void QtChatWidget::emoticonSelected(QtEmoticon emoticon) {
	_chatEdit->insertHtml(emoticon.getHtml());
	_chatEdit->ensureCursorVisible();
}

QString QtChatWidget::getNewBackgroundColor() const {
	_lastBackGroundColor.setRed(_lastBackGroundColor.red() + 20);
	return QString("%1").arg(_lastBackGroundColor.name());
}

void QtChatWidget::avatarFrameButtonClicked() {
	if (_isAvatarFrameOpened) {
		removeAvatarFrame();
	} else {
		addAvatarFrame();
	}
}

void QtChatWidget::enterPressed(Qt::KeyboardModifiers modifier) {
	//CTRL+ENTER = new line
	if (modifier == Qt::ControlModifier) {
		_chatEdit->append(QString::null);
		////
	} else {
		sendMessage();
	}
}

void QtChatWidget::addAvatarFrame() {
	QBoxLayout * glayout = dynamic_cast<QBoxLayout *>(layout());
	_avatarFrame->setVisible(true);
	//_avatarFrame->setMinimumSize(64, 0);
	glayout->insertWidget(1, _avatarFrame);

	_isAvatarFrameOpened = true;
	_ui.avatarFrameButton->setIcon(QIcon(":images/chat/show_avatar_frame.png"));
	update();
}

void QtChatWidget::removeAvatarFrame() {
	QBoxLayout * glayout = dynamic_cast<QBoxLayout *>(layout());
	_avatarFrame->setVisible(false);
	_avatarFrame->setMinimumSize(0, 0);
	glayout->removeWidget(_avatarFrame);

	_isAvatarFrameOpened = false;
	_ui.avatarFrameButton->setIcon(QIcon(":images/chat/hide_avatar_frame.png"));
	update();
}

void QtChatWidget::addToHistory(const QString & senderName, const QString & str) {
	QTextCursor curs(_chatHistory->document());
	curs.movePosition(QTextCursor::End);
	_chatHistory->setTextCursor(curs);

	if (!hasUserColor(senderName)) {
		_userColorHash[senderName] = getNewBackgroundColor();
	}

	// insert "header" code html
	QString header = QtChatUtils::getHeader(getUserColor(senderName),
		CHAT_USER_FOREGROUND_COLOR, senderName);
	_chatHistory->insertHtml(header);
	////

	// insert message html code
	QString table = QString("<table border=\"0\" width=\"98%\" cellspacing=\"0\" cellpadding=\"5\">") +
		QString("<tr><td>") +
		//QtChatUtils::decodeMessage(_imChatSession->getIMChat().getIMAccount().getProtocol(), str) +
		QString("</td></tr></table>");
	_chatHistory->insertHtml(table);
	////

	_chatHistory->ensureCursorVisible();
}

void QtChatWidget::addStatusMessage(const QString & statusMessage) {
	QString message = QtChatUtils::getHeader(CHAT_USER_BACKGOUND_COLOR,
		"#888888", "<i>" + statusMessage + "</i>");
	_chatHistory->insertHtml(message);
}

void QtChatWidget::sendMessage() {

	//manage typing timer
	stopNotTypingTimer();
	stopStoppedTypingTimer();
	////

	//Not typing anymore
	//_imChatSession->changeTypingState(IMChat::TypingStateNotTyping);

	//Drop empty message
	if (_chatEdit->toPlainText().isEmpty()) {
		return;
	}

	//add the text to QtChatHistoryWidget
	addToHistory(_nickName, _chatEdit->toHtml());
	////

	//std::string message(QtChatUtils::encodeMessage(
	//	_chatEdit->currentFont(),
	//	_imChatSession->getIMChat().getIMAccount().getProtocol(),
	//	_chatEdit->toHtml()).toUtf8().constData()
	//);
	//_imChatSession->sendMessage(message);
	////

	_chatEdit->clear();
	_chatEdit->setCurrentFont(_currentFont);
	_chatEdit->setTextColor(_currentColor);
	_chatEdit->setFontItalic(_italic);
	_chatEdit->setFontUnderline(_underline);
	if (_bold) {
		_chatEdit->setFontWeight(QFont::Bold);
	} else {
		_chatEdit->setFontWeight(QFont::Normal);
	}
	_chatEdit->setFocus();
	_chatEdit->ensureCursorVisible();
	_chatEdit->insertPlainText(" ");
	_chatEdit->setPlainText("");
}



void QtChatWidget::chatEditTextChanged() {
	// change typing state to Typing
	if (!_isTyping) {
		//_imChatSession->changeTypingState(IMChat::TypingStateTyping);
		_isTyping = true;
	}
	////

	// manage timers
	stopNotTypingTimer();
	if (_stoppedTypingTimerId == -1) {
		_stoppedTypingTimerId = startTimer(CHAT_STOPPED_TYPING_DELAY);
	} else {
		killTimer(_stoppedTypingTimerId);
		_stoppedTypingTimerId = startTimer(CHAT_STOPPED_TYPING_DELAY);
	}
	////
}

void QtChatWidget::timerEvent(QTimerEvent * event) {
	if (event->timerId() == _stoppedTypingTimerId) {
		killTimer(_stoppedTypingTimerId);
		_stoppedTypingTimerId = -1;
		//_imChatSession->changeTypingState(IMChat::TypingStateStopTyping);
		_notTypingTimerId = startTimer(CHAT_NOT_TYPING_DELAY);
		_isTyping = false;
	}

	if (event->timerId() == _notTypingTimerId) {
		killTimer(_notTypingTimerId);
		_notTypingTimerId = -1;
		//_imChatSession->changeTypingState(IMChat::TypingStateNotTyping);
		_isTyping = false;
	}
}

void QtChatWidget::stopNotTypingTimer() {
	if (_notTypingTimerId != -1) {
		killTimer(_notTypingTimerId);
		_notTypingTimerId = -1;
	}
}

void QtChatWidget::stopStoppedTypingTimer() {
	if (_stoppedTypingTimerId != -1) {
		killTimer(_stoppedTypingTimerId);
		_stoppedTypingTimerId = -1;
	}
}

//Work around for Qt 4.1.2 bug, can be removed with Qt > 4.1.2
void QtChatWidget::deletePressed() {
	QTextCursor cursor = _chatEdit->textCursor();
	if (!cursor.hasSelection()) {
		cursor.movePosition( QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, 1 );
		cursor.removeSelectedText();
	} else {
		cursor.removeSelectedText();
	}
	_chatEdit->setTextCursor(cursor);
}
////

//void QtChatWidget::updateAvatarFrame() {
//	QMutexLocker locker(&_mutex);

	//QtContactList * qtContactList = _qtrsphone->getQtContactList();
	//CContactList & cContactList = qtContactList->getCContactList();

	//IMContactSet imContactSet = _imChatSession->getIMContactSet();
	//IMContactSet::iterator it;
	//for (it = imContactSet.begin(); it != imContactSet.end(); it++) {

		//std::string contactId = cContactList.findContactThatOwns(*it);
		//ContactProfile profile = cContactList.getContactProfile(contactId);

		//std::string data = profile.getIcon().getData();
		//QPixmap pixmap;
		//pixmap.loadFromData((uchar *)data.c_str(), data.size());
		//_avatarFrame->addRemoteContact(
			//QString::fromStdString(contactId),
			//QString::fromStdString(profile.getDisplayName()),
			//QString::fromStdString((*it).getContactId()),
			//pixmap
		//);
	//}
//}

void QtChatWidget::updateUserAvatar() {

	QPixmap pixmap;
	//std::string myData = _cChatHandler.getCUserProfile().getUserProfile().getIcon().getData();
	//pixmap.loadFromData((uchar *)myData.c_str(), myData.size());
	_avatarFrame->setUserPixmap(pixmap);
}


void QtChatWidget::show()
   {
    //loadSettings();
    if(!this->isVisible()) {
      QDialog::show();
   
     }
   }
   


