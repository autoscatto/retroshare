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
  
  //m_chatwidget = new QtChatWidget(this); 

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
      connect( connectfriendAct , SIGNAL( triggered() ), this, SLOT( connectfriend() ) );
      
      configurefriendAct = new QAction( tr( "Configure Friend" ), this );
      connect( configurefriendAct , SIGNAL( triggered() ), this, SLOT( configurefriend() ) );
      
      exportfriendAct = new QAction(QIcon(IMAGE_EXPIORTFRIEND), tr( "Export Friend" ), this );
      connect( exportfriendAct , SIGNAL( triggered() ), this, SLOT( exportfriend() ) );
      
      removefriendAct = new QAction(QIcon(IMAGE_REMOVEFRIEND), tr( "Remove Friend" ), this );
      connect( removefriendAct , SIGNAL( triggered() ), this, SLOT( removefriend() ) );


      contextMnu.clear();
      contextMnu.addAction( chatAct);
      contextMnu.addAction( connectfriendAct);
      contextMnu.addAction( configurefriendAct);
      contextMnu.addAction( exportfriendAct);
      contextMnu.addAction( removefriendAct);
      contextMnu.exec( mevent->globalPos() );
}



/* get the list of peers from the RsIface.  */
void  MessengerWindow::insertPeers2()
{
        rsiface->lockData(); /* Lock Interface */

        std::map<RsCertId,NeighbourInfo>::const_iterator it;
        const std::map<RsCertId,NeighbourInfo> &friends =
                                rsiface->getFriendMap();

        /* get a link to the table */
        QTreeWidget *messengertreeWidget = ui.messengertreeWidget;

        /* remove old items ??? */
	messengertreeWidget->clear();
	messengertreeWidget->setColumnCount(2);

        QList<QTreeWidgetItem *> items;
	for(it = friends.begin(); it != friends.end(); it++)
	{
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* First 5 (0-4) Key Items */
		/* (0) Status */
		item -> setText(0, QString::fromStdString(
						it->second.statusString));

		/* (1) Person */
		item -> setText(1, QString::fromStdString(it->second.name));

		/* (2) Auto Connect */
		//item -> setText(2, QString::fromStdString(
		//				it->second.connectString));

		/* (3) Trust Level */
		//item -> setText(3, QString::fromStdString(it->second.trustString));
		/* (4) Peer Address */
		//item -> setText(4, QString::fromStdString(it->second.peerAddress));

		/* less important ones */
		/* () Last Contact */
		//item -> setText(5, QString::fromStdString(it->second.lastConnect));

		/* () Org */
		//item -> setText(6, QString::fromStdString(it->second.org));
		/* () Location */
		//item -> setText(7, QString::fromStdString(it->second.loc));
		/* () Country */
		//item -> setText(8, QString::fromStdString(it->second.country));
	
		/* Hidden ones: */
		/* ()  RsCertId */
		//{
		//	std::ostringstream out;
		//	out << it -> second.id;
		//	item -> setText(9, QString::fromStdString(out.str()));
		//}

		/* ()  AuthCode */	
        //        item -> setText(10, QString::fromStdString(it->second.authCode));

		/* add to the list */
		//items.append(item);
	}

	/* add the items in! */
	messengertreeWidget->insertTopLevelItems(0, items);

	rsiface->unlockData(); /* UnLock Interface */

	messengertreeWidget->update(); /* update display */
}

/* Utility Fns */
std::string getPeer2RsCertId(QTreeWidgetItem *i)
{
	std::string id = (i -> text(9)).toStdString();
	return id;
}

/** Open a QFileDialog to browse for export a file. */
void MessengerWindow::exportfriend2()
{
        QTreeWidgetItem *c = getCurrentPeer2();
        std::cerr << "MessengerWindow::exportfriend()" << std::endl;
	if (!c)
	{
        	std::cerr << "MessengerWindow::exportfriend() Noone Selected -- sorry" << std::endl;
		return;
	}

	std::string id = getPeer2RsCertId(c);
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Certificate"), "",
	                                                     tr("Certificates (*.pqi)"));

	std::string file = fileName.toStdString();
	if (file != "")
	{
        	std::cerr << "MessengerWindow::exportfriend() Saving to: " << file << std::endl;
        	std::cerr << std::endl;
		rsicontrol->FriendSaveCertificate(id, file);
	}

}

void MessengerWindow::chatfriend2()
{
    //static PopupChatDialog *popupchatdialog = new PopupChatDialog();
    //popupchatdialog->show();
    

   

}



QTreeWidgetItem *MessengerWindow::getCurrentPeer2()
{
	/* get the current, and extract the Id */

	/* get a link to the table */
        QTreeWidget *messengertreeWidget = ui.messengertreeWidget;
        QTreeWidgetItem *item = messengertreeWidget -> currentItem();
        if (!item)
        {
		std::cerr << "Invalid Current Item" << std::endl;
		return NULL;
	}

	/* Display the columns of this item. */
	std::ostringstream out;
        out << "CurrentPeerItem: " << std::endl;

	for(int i = 0; i < 5; i++)
	{
		QString txt = item -> text(i);
		out << "\t" << i << ":" << txt.toStdString() << std::endl;
	}
	std::cerr << out.str();
	return item;
}

/* So from the Peers Dialog we can call the following control Functions:
 * (1) Remove Current.              FriendRemove(id)
 * (2) Allow/DisAllow.              FriendStatus(id, accept)
 * (2) Connect.                     FriendConnectAttempt(id, accept)
 * (3) Set Address.                 FriendSetAddress(id, str, port)
 * (4) Set Trust.                   FriendTrustSignature(id, bool)
 * (5) Configure (GUI Only) -> 3/4
 *
 * All of these rely on the finding of the current Id.
 */
 

void MessengerWindow::removefriend2()
{
        QTreeWidgetItem *c = getCurrentPeer2();
        std::cerr << "MessengerWindow::removefriend()" << std::endl;
	if (!c)
	{
        	std::cerr << "MessengerWindow::removefriend() Noone Selected -- sorry" << std::endl;
		return;
	}
	rsicontrol->FriendRemove(getPeer2RsCertId(c));
}


void MessengerWindow::allowfriend2()
{
	QTreeWidgetItem *c = getCurrentPeer2();
	std::cerr << "MessengerWindow::allowfriend()" << std::endl;
	/*
	bool accept = true;
	rsServer->FriendStatus(getPeerRsCertId(c), accept);
	*/
}


void MessengerWindow::connectfriend2()
{
	QTreeWidgetItem *c = getCurrentPeer2();
	std::cerr << "MessengerWindow::connectfriend()" << std::endl;
	rsicontrol->FriendConnectAttempt(getPeer2RsCertId(c));
}

void MessengerWindow::setaddressfriend2()
{
	QTreeWidgetItem *c = getCurrentPeer2();
	std::cerr << "MessengerWindow::setaddressfriend()" << std::endl;

	/* need to get the input address / port */
	/*
 	std::string addr;
	unsigned short port;
	rsServer->FriendSetAddress(getPeerRsCertId(c), addr, port);
	*/
}

void MessengerWindow::trustfriend2()
{
	QTreeWidgetItem *c = getCurrentPeer2();
	std::cerr << "MessengerWindow::trustfriend()" << std::endl;
	/*
	bool trust = true;
	rsServer->FriendTrust(getPeerRsCertId(c), trust);
	*/
}



/* GUI stuff -> don't do anything directly with Control */
void MessengerWindow::configurefriend2()
{
	/* display Dialog */
	std::cerr << "MessengerWindow::configurefriend()" << std::endl;
	QTreeWidgetItem *c = getCurrentPeer2();


	static ConfCertDialog *confdialog = new ConfCertDialog();


	if (!c)
		return;

	/* set the Id */
	std::string id = getPeer2RsCertId(c);

	confdialog -> loadId(id);
	confdialog -> show();
}

void MessengerWindow::closeEvent (QCloseEvent * event)
{

 QWidget::closeEvent(event);
}

