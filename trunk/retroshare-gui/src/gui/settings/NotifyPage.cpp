/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006 - 2009 RetroShare Team
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


#include <rshare.h>
#include "NotifyPage.h"
#include <iostream>
#include <sstream>

#include <retroshare/rsnotify.h>
#include "rsharesettings.h"

#include "gui/MainWindow.h"

#include <QTimer>


/** Constructor */
NotifyPage::NotifyPage(QWidget * parent, Qt::WFlags flags)
  : ConfigPage(parent, flags)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);
  setAttribute(Qt::WA_QuitOnClose, false);
  setWindowTitle(windowTitle() + QLatin1String(" - Notify"));




  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

NotifyPage::~NotifyPage()
{
}

void
NotifyPage::closeEvent (QCloseEvent * event)
{
    QWidget::closeEvent(event);
}

/** Saves the changes on this page */
bool
NotifyPage::save(QString &errmsg)
{
    /* extract from rsNotify the flags */

    uint notifyflags = 0;
    uint newsflags   = 0;
    uint chatflags   = 0;

    if (ui.popup_Connect->isChecked())
        notifyflags |= RS_POPUP_CONNECT;

    if (ui.popup_NewMsg->isChecked())
        notifyflags |= RS_POPUP_MSG;

    //if (ui.popup_NewChat->isChecked())
    notifyflags |= RS_POPUP_CHAT;

    //if (ui.popup_Call->isChecked())
    //	notifyflags |= RS_POPUP_CALL;


    if (ui.notify_Peers->isChecked())
        newsflags |= RS_FEED_TYPE_PEER;
    if (ui.notify_Channels->isChecked())
        newsflags |= RS_FEED_TYPE_CHAN;
    if (ui.notify_Forums->isChecked())
        newsflags |= RS_FEED_TYPE_FORUM;
    if (ui.notify_Blogs->isChecked())
        newsflags |= RS_FEED_TYPE_BLOG;
    if (ui.notify_Chat->isChecked())
        newsflags |= RS_FEED_TYPE_CHAT;
    if (ui.notify_Messages->isChecked())
        newsflags |= RS_FEED_TYPE_MSG;
    if (ui.notify_Chat->isChecked())
        newsflags |= RS_FEED_TYPE_CHAT;

    if (ui.chat_NewWindow->isChecked())
        chatflags |= RS_CHAT_OPEN_NEW;
    if (ui.chat_Reopen->isChecked())
        chatflags |= RS_CHAT_REOPEN;
    if (ui.chat_Focus->isChecked())
        chatflags |= RS_CHAT_FOCUS;

    Settings->setNotifyFlags(notifyflags);
    Settings->setNewsFeedFlags(newsflags);
    Settings->setChatFlags(chatflags);

    Settings->setDisplayTrayGroupChat(ui.systray_GroupChat->isChecked());
    MainWindow::installGroupChatNotifier();

    load();
    return true;
}


/** Loads the settings for this page */
void NotifyPage::load()
{
    /* extract from rsNotify the flags */
    uint notifyflags = Settings->getNotifyFlags();
    uint newsflags = Settings->getNewsFeedFlags();
    uint chatflags = Settings->getChatFlags();

    ui.popup_Connect->setChecked(notifyflags & RS_POPUP_CONNECT);
    ui.popup_NewMsg->setChecked(notifyflags & RS_POPUP_MSG);
    ui.systray_GroupChat->setChecked(true) ;
    //ui.popup_NewChat->setChecked(notifyflags & RS_POPUP_CHAT);
    //ui.popup_Call->setChecked(notifyflags & RS_POPUP_CALL);

    ui.notify_Peers->setChecked(newsflags & RS_FEED_TYPE_PEER);
    ui.notify_Channels->setChecked(newsflags & RS_FEED_TYPE_CHAN);
    ui.notify_Forums->setChecked(newsflags & RS_FEED_TYPE_FORUM);
    ui.notify_Blogs->setChecked(newsflags & RS_FEED_TYPE_BLOG);
    ui.notify_Chat->setChecked(newsflags & RS_FEED_TYPE_CHAT);
    ui.notify_Messages->setChecked(newsflags & RS_FEED_TYPE_MSG);
    ui.notify_Chat->setChecked(newsflags & RS_FEED_TYPE_CHAT);

    ui.chat_NewWindow->setChecked(chatflags & RS_CHAT_OPEN_NEW);
    ui.chat_Reopen->setChecked(chatflags & RS_CHAT_REOPEN);
    ui.chat_Focus->setChecked(chatflags & RS_CHAT_FOCUS);

    ui.systray_GroupChat->setChecked(Settings->getDisplayTrayGroupChat());

    /* disable ones that don't work yet */

}


/** Loads the settings for this page */
void NotifyPage::updateStatus()
{
}
