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
#include "common/vmessagebox.h"

#include "rshare.h"
#include "MessengerWindow.h"
#include "rsiface/rsiface.h"
#include "chat/PopupChatDialog.h"
#include "ChatDialog.h"
#include "connect/ConfCertDialog.h"

#include <iostream>
#include <sstream>


#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>


/* Images for context menu icons */
#define IMAGE_REMOVEFRIEND       ":/images/removefriend16.png"
#define IMAGE_EXPIORTFRIEND      ":/images/exportpeers_16x16.png"
#define IMAGE_CHAT               ":/images/chat.png"

/** Constructor */
MessengerWindow::MessengerWindow(QWidget * parent)
: QWidget(parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);
  

  connect( ui.messengertreeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( messengertreeWidgetCostumPopupMenu( QPoint ) ) );


  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void MessengerWindow::messengertreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      chatAct = new QAction(QIcon(IMAGE_CHAT), tr( "Chat" ), this );
      connect( chatAct , SIGNAL( triggered() ), this, SLOT( chatfriend2() ) );

      connectfriendAct = new QAction( tr( "Connect To Friend" ), this );
      connect( connectfriendAct , SIGNAL( triggered() ), this, SLOT( connectfriend2() ) );
     
     /************ Do we want these options here??? 
      *
      *
      configurefriendAct = new QAction( tr( "Configure Friend" ), this );
      connect( configurefriendAct , SIGNAL( triggered() ), this, SLOT( configurefriend2() ) );
      
      exportfriendAct = new QAction(QIcon(IMAGE_EXPIORTFRIEND), tr( "Export Friend" ), this );
      connect( exportfriendAct , SIGNAL( triggered() ), this, SLOT( exportfriend2() ) );
      
      removefriendAct = new QAction(QIcon(IMAGE_REMOVEFRIEND), tr( "Remove Friend" ), this );
      connect( removefriendAct , SIGNAL( triggered() ), this, SLOT( removefriend2() ) );
       *
       *
      *********/

      contextMnu.clear();
      contextMnu.addAction( chatAct);
      contextMnu.addAction( connectfriendAct);

      /**** Do we want these options here??? 
       *
       *
      contextMnu.addAction( configurefriendAct);
      contextMnu.addAction( exportfriendAct);
      contextMnu.addAction( removefriendAct);
       *
       *
      ****/

      contextMnu.exec( mevent->globalPos() );
}



/* get the list of peers from the RsIface.  */
void  MessengerWindow::insertPeers()
{
        rsiface->lockData(); /* Lock Interface */

        std::map<RsCertId,NeighbourInfo>::const_iterator it;
        const std::map<RsCertId,NeighbourInfo> &friends =
                                rsiface->getFriendMap();

        /* get a link to the table */
        QTreeWidget *peerWidget = ui.messengertreeWidget;

        /* remove old items ??? */
	peerWidget->clear();
	peerWidget->setColumnCount(4);


	/* have two lists: online / offline */
        QList<QTreeWidgetItem *> online_items;
        QList<QTreeWidgetItem *> offline_items;


	for(it = friends.begin(); it != friends.end(); it++)
	{
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* (0) Person */
		item -> setText(0, QString::fromStdString(it->second.name));
		/* (1) Org */
		item -> setText(1, QString::fromStdString(it->second.org));
		/* (2) Location */
		item -> setText(2, QString::fromStdString(it->second.loc));
		/* (3) Country */
		item -> setText(3, QString::fromStdString(it->second.country));

		/* Hidden ones: */
		/* ()  RsCertId */
		{
			std::ostringstream out;
			out << it -> second.id;
			item -> setText(4, QString::fromStdString(out.str()));
		}

		/* add to the list */
                if (it->second.statusString == "Online")
		{
		   online_items.append(item);
		}
		else
		{
		   offline_items.append(item);
		}
	}

	/* make parent items (TODO) */
	/* add the items in! */

	if (online_items.size() > 0)
	{
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* (0) Person */
		item -> setText(0, "Online");
		item -> addChildren(online_items);
	        peerWidget->addTopLevelItem(item);
		peerWidget->expandItem(item);
	}

	if (offline_items.size() > 0)
	{
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* (0) Person */
		item -> setText(0, "Offline");
		item -> addChildren(offline_items);
	        peerWidget->addTopLevelItem(item);
		peerWidget->expandItem(item);
	}

	rsiface->unlockData(); /* UnLock Interface */

	peerWidget->update(); /* update display */
}


/* Utility Fns */
std::string getMessengerPeerRsCertId(QTreeWidgetItem *i)
{
	std::string id = (i -> text(4)).toStdString();
	return id;
}


/** Open a QFileDialog to browse for export a file. */
void MessengerWindow::exportfriend2()
{

}

void MessengerWindow::removefriend2()
{
   
}


void MessengerWindow::allowfriend2()
{
	
}


void MessengerWindow::connectfriend2()
{
    bool isOnline;
    QTreeWidgetItem *i = getCurrentPeer(isOnline);
    if (!i)
      return;

    if (isOnline)
    {
	std::cerr << "MessengerWindow::connectfriend2() Already online" << std::endl;
    }
    else
    {
	std::cerr << "MessengerWindow::connectfriend2() Trying" << std::endl;
	rsicontrol->FriendConnectAttempt(getMessengerPeerRsCertId(i));
    }
}

void MessengerWindow::setaddressfriend2()
{
	
}

void MessengerWindow::trustfriend2()
{
	
}



/* GUI stuff -> don't do anything directly with Control */
void MessengerWindow::configurefriend2()
{
	
}

void MessengerWindow::closeEvent (QCloseEvent * event)
{

 QWidget::closeEvent(event);
}



void MessengerWindow::setChatDialog(ChatDialog *cd)
{
  chatDialog = cd;
}




void MessengerWindow::chatfriend2()
{
    bool isOnline;
    QTreeWidgetItem *i = getCurrentPeer(isOnline);
    if (!i)
      return;

    std::string name = (i -> text(0)).toStdString();
    std::string id = (i -> text(4)).toStdString();

    if (!isOnline)
    {
    	/* info dialog */
        QMessageBox::StandardButton sb = QMessageBox::question ( NULL, 
			"Friend Not Online", 
	"Your Friend is offline \nWhy don't you send them a Message instead",
	(QMessageBox::Ok | QMessageBox::Reset ));
	if (sb == QMessageBox::Reset)
	{
		while(QMessageBox::Yes == QMessageBox::question ( NULL,         
	                        "Rhetorical Question",
			        "Are You Sure?",
			        (QMessageBox::Yes | QMessageBox::No)));
	}
	return;
    }

    /* must reference ChatDialog */
    if (chatDialog)
    {
    	chatDialog->getPrivateChat(id, name, true);
    }
}


QTreeWidgetItem *MessengerWindow::getCurrentPeer(bool &isOnline)
{
	/* get the current, and extract the Id */

	/* get a link to the table */
        QTreeWidget *peerWidget = ui.messengertreeWidget;
        QTreeWidgetItem *item = peerWidget -> currentItem();
        if (!item)
        {
		std::cerr << "Invalid Current Item" << std::endl;
		return NULL;
	}

	/* check if parent is online or offline */
        QTreeWidgetItem *parent = item->parent();
	if ((!parent) ||
	    (parent == peerWidget->invisibleRootItem()))
	{
		std::cerr << "Selected Parent Invalid Item" << std::endl;
		return NULL;
	}

	isOnline = (parent->text(0) == "Online");

	return item;
}






