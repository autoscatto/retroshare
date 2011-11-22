/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006, crypton
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

#include <QFile>
#include <QFileInfo>
#include <QWidgetAction>
#include <QTimer>

#include "common/vmessagebox.h"
#include "common/StatusDefs.h"

#include <retroshare/rspeers.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsstatus.h>
#include <retroshare/rsnotify.h>

#include "rshare.h"
#include "MessengerWindow.h"
#include "RsAutoUpdatePage.h"

#ifndef MINIMAL_RSGUI
#include "MainWindow.h"
#include "ShareManager.h"
#include "notifyqt.h"
#include "connect/ConnectFriendWizard.h"
#endif // MINIMAL_RSGUI
#include "util/PixmapMerging.h"
#include "LogoBar.h"
#include "util/Widget.h"
#include "util/misc.h"
#include "settings/rsharesettings.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <set>


/******
 * #define MSG_DEBUG 1
 *****/

MessengerWindow* MessengerWindow::_instance = NULL;
static std::set<std::string> *expandedPeers = NULL;
static std::set<std::string> *expandedGroups = NULL;

/*static*/ void MessengerWindow::showYourself ()
{
    if (_instance == NULL) {
        _instance = new MessengerWindow();
    }

    _instance->show();
    _instance->activateWindow();
}

MessengerWindow* MessengerWindow::getInstance()
{
    return _instance;
}

void MessengerWindow::releaseInstance()
{
    if (_instance) {
        delete _instance;
    }
    if (expandedPeers) {
        /* delete saved expanded peers */
        delete(expandedPeers);
        expandedPeers = NULL;
    }
    if (expandedGroups) {
        /* delete saved expanded groups */
        delete(expandedGroups);
        expandedGroups = NULL;
    }
}

/** Constructor */
MessengerWindow::MessengerWindow(QWidget* parent, Qt::WFlags flags)
    : 	RWindow("MessengerWindow", parent, flags)
{
    /* Invoke the Qt Designer generated object setup routine */
    ui.setupUi(this);

    setAttribute ( Qt::WA_DeleteOnClose, true );
#ifdef MINIMAL_RSGUI
    setAttribute (Qt::WA_QuitOnClose, true);
#endif // MINIMAL_RSGUI

    ui.avatar->setFrameType(AvatarWidget::STATUS_FRAME);
    ui.avatar->setOwnId();

#ifndef MINIMAL_RSGUI
    connect( ui.shareButton, SIGNAL(clicked()), SLOT(openShareManager()));
    connect( ui.addIMAccountButton, SIGNAL(clicked( bool ) ), this , SLOT( addFriend() ) );
#endif // MINIMAL_RSGUI
    connect(ui.clearButton, SIGNAL(clicked()), this, SLOT(clearFilter()));

    connect(ui.messagelineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(savestatusmessage()));
    connect(ui.filterPatternLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(filterRegExpChanged()));

#ifndef MINIMAL_RSGUI
    connect(NotifyQt::getInstance(), SIGNAL(ownAvatarChanged()), this, SLOT(updateAvatar()));
    connect(NotifyQt::getInstance(), SIGNAL(ownStatusMessageChanged()), this, SLOT(loadmystatusmessage()));
    connect(NotifyQt::getInstance(), SIGNAL(peerStatusChanged(QString,int)), this, SLOT(updateOwnStatus(QString,int)));
#endif // MINIMAL_RSGUI

    if (expandedPeers != NULL) {
        for (std::set<std::string>::iterator peerIt = expandedPeers->begin(); peerIt != expandedPeers->end(); peerIt++) {
            ui.friendList->addPeerToExpand(*peerIt);
        }
        delete expandedPeers;
        expandedPeers = NULL;
    }

    if (expandedGroups != NULL) {
        for (std::set<std::string>::iterator groupIt = expandedGroups->begin(); groupIt != expandedGroups->end(); groupIt++) {
            ui.friendList->addGroupToExpand(*groupIt);
        }
        delete expandedGroups;
        expandedGroups = NULL;
    }

    //LogoBar
    _rsLogoBarmessenger = new LogoBar(ui.logoframe);
    Widget::createLayout(ui.logoframe)->addWidget(_rsLogoBarmessenger);

    ui.messagelineEdit->setMinimumWidth(20);

    ui.displaypushButton->setMenu(ui.friendList->createDisplayMenu());

    // load settings
    RsAutoUpdatePage::lockAllEvents();
    ui.friendList->setShowStatusColumn(false);
    ui.friendList->setShowLastContactColumn(false);
    ui.friendList->setShowAvatarColumn(true);
    ui.friendList->setRootIsDecorated(true);
    ui.friendList->setShowGroups(false);
    processSettings(true);
    ui.friendList->setBigName(true);
    RsAutoUpdatePage::unlockAllEvents();

    // add self nick
    RsPeerDetails pd;
    std::string ownId = rsPeers->getOwnId();
    if (rsPeers->getPeerDetails(ownId, pd)) {
        /* calculate only once */
        m_nickName = QString::fromUtf8(pd.name.c_str());
#ifdef MINIMAL_RSGUI
         ui.statusButton->setText(m_nickName);
#endif
    }

#ifndef MINIMAL_RSGUI
    /* Show nick and current state */
    StatusInfo statusInfo;
    rsStatus->getOwnStatus(statusInfo);
    updateOwnStatus(QString::fromStdString(ownId), statusInfo.status);

    MainWindow *pMainWindow = MainWindow::getInstance();
    if (pMainWindow) {
        QMenu *pStatusMenu = new QMenu();
        pMainWindow->initializeStatusObject(pStatusMenu, true);
        ui.statusButton->setMenu(pStatusMenu);
    }

    loadmystatusmessage();
#endif // MINIMAL_RSGUI

    ui.clearButton->hide();

    /* Hide platform specific features */
#ifdef Q_WS_WIN
#endif
}

MessengerWindow::~MessengerWindow ()
{
    // save settings
    processSettings(false);

#ifndef MINIMAL_RSGUI
    MainWindow *pMainWindow = MainWindow::getInstance();
    if (pMainWindow) {
        pMainWindow->removeStatusObject(ui.statusButton);
    }
#endif // MINIMAL_RSGUI

    _instance = NULL;
}

void MessengerWindow::processSettings(bool bLoad)
{
    Settings->beginGroup(_name);
    ui.friendList->processSettings(bLoad);
    Settings->endGroup();
}

#ifndef MINIMAL_RSGUI
/** Add a Friend ShortCut */
void MessengerWindow::addFriend()
{
    ConnectFriendWizard connwiz (this);

    connwiz.exec ();
}
#endif

//============================================================================

void MessengerWindow::closeEvent (QCloseEvent * /*event*/)
{
    /* save the expanded peers */
    if (expandedPeers == NULL) {
        expandedPeers = new std::set<std::string>;
    } else {
        expandedPeers->clear();
    }

    ui.friendList->getExpandedPeers(*expandedPeers);

    /* save the expanded groups */
    if (expandedGroups == NULL) {
        expandedGroups = new std::set<std::string>;
    } else {
        expandedGroups->clear();
    }

    ui.friendList->getExpandedGroups(*expandedGroups);
}

LogoBar & MessengerWindow::getLogoBar() const {
        return *_rsLogoBarmessenger;
}

#ifndef MINIMAL_RSGUI
/** Shows Share Manager */
void MessengerWindow::openShareManager()
{
	ShareManager::showYourself();
}

/** Loads own personal status message */
void MessengerWindow::loadmystatusmessage()
{ 
    ui.messagelineEdit->setEditText( QString::fromUtf8(rsMsgs->getCustomStateString().c_str()));
}

/** Save own status message */
void MessengerWindow::savestatusmessage()
{
    rsMsgs->setCustomStateString(ui.messagelineEdit->currentText().toUtf8().constData());
}

void MessengerWindow::updateOwnStatus(const QString &peer_id, int status)
{
    // add self nick + own status
    if (peer_id.toStdString() == rsPeers->getOwnId())
    {
        // my status has changed

        ui.statusButton->setText(m_nickName + " (" + StatusDefs::name(status) + ")");

        return;
    }
}

#endif // MINIMAL_RSGUI

/* clear Filter */
void MessengerWindow::clearFilter()
{
    ui.filterPatternLineEdit->clear();
    ui.filterPatternLineEdit->setFocus();
}

void MessengerWindow::filterRegExpChanged()
{

    QString text = ui.filterPatternLineEdit->text();

    if (text.isEmpty()) {
        ui.clearButton->hide();
    } else {
        ui.clearButton->show();
    }

    ui.friendList->filterItems(text);
}