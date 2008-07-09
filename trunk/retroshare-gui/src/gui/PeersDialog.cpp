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

#include <QFile>
#include <QFileInfo>
#include "common/vmessagebox.h"

#include "rshare.h"
#include "PeersDialog.h"
#include "rsiface/rsiface.h"
#include "rsiface/rspeers.h"
#include "rsiface/rsstatus.h"

#include "chat/PopupChatDialog.h"
#include "msgs/ChanMsgDialog.h"
#include "ChatDialog.h"
#include "connect/ConfCertDialog.h"

#include <iostream>
#include <sstream>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QMessageBox>
#include <QHeaderView>


/* Images for context menu icons */
#define IMAGE_REMOVEFRIEND       ":/images/removefriend16.png"
#define IMAGE_EXPIORTFRIEND      ":/images/exportpeers_16x16.png"
#define IMAGE_PEERINFO           ":/images/peerdetails_16x16.png"
#define IMAGE_CHAT               ":/images/chat.png"
#define IMAGE_MSG                ":/images/message-mail.png"
/* Images for Status icons */
#define IMAGE_ONLINE             ":/images/donline.png"
#define IMAGE_OFFLINE            ":/images/dhidden.png"

/******
 * #define PEERS_DEBUG 1
 *****/


/** Constructor */
PeersDialog::PeersDialog(QWidget *parent)
: MainPage(parent), chatDialog(NULL)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.peertreeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( peertreeWidgetCostumPopupMenu( QPoint ) ) );

  /* hide the Tree +/- */
  ui.peertreeWidget -> setRootIsDecorated( false );
  
    /* Set header resize modes and initial section sizes */
	QHeaderView * _header = ui.peertreeWidget->header () ;
   	_header->setResizeMode (0, QHeaderView::Custom);
	_header->setResizeMode (1, QHeaderView::Interactive);
	_header->setResizeMode (2, QHeaderView::Interactive);
	_header->setResizeMode (3, QHeaderView::Interactive);
	_header->setResizeMode (4, QHeaderView::Interactive);
	_header->setResizeMode (5, QHeaderView::Interactive);
	_header->setResizeMode (6, QHeaderView::Interactive);
	_header->setResizeMode (7, QHeaderView::Interactive);

    
	_header->resizeSection ( 0, 25 );
	_header->resizeSection ( 1, 100 );
	_header->resizeSection ( 2, 100 );
	_header->resizeSection ( 3, 120 );
	_header->resizeSection ( 4, 100 );
	_header->resizeSection ( 5, 230 );
	_header->resizeSection ( 6, 120 );
	_header->resizeSection ( 7, 220 );



  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void PeersDialog::setChatDialog(ChatDialog *cd)
{
  chatDialog = cd;
}


void PeersDialog::peertreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      chatAct = new QAction(QIcon(IMAGE_CHAT), tr( "Chat" ), this );
      connect( chatAct , SIGNAL( triggered() ), this, SLOT( chatfriend() ) );

      msgAct = new QAction(QIcon(IMAGE_MSG), tr( "Message Friend" ), this );
      connect( msgAct , SIGNAL( triggered() ), this, SLOT( msgfriend() ) );

      connectfriendAct = new QAction( tr( "Connect To Friend" ), this );
      connect( connectfriendAct , SIGNAL( triggered() ), this, SLOT( connectfriend() ) );
      
      configurefriendAct = new QAction(QIcon(IMAGE_PEERINFO), tr( "Peer Details" ), this );
      connect( configurefriendAct , SIGNAL( triggered() ), this, SLOT( configurefriend() ) );
      
      exportfriendAct = new QAction(QIcon(IMAGE_EXPIORTFRIEND), tr( "Export Friend" ), this );
      connect( exportfriendAct , SIGNAL( triggered() ), this, SLOT( exportfriend() ) );
      
      removefriendAct = new QAction(QIcon(IMAGE_REMOVEFRIEND), tr( "Remove Friend" ), this );
      connect( removefriendAct , SIGNAL( triggered() ), this, SLOT( removefriend() ) );


      contextMnu.clear();
      contextMnu.addAction( chatAct);
      contextMnu.addAction( msgAct);
      contextMnu.addSeparator(); 
      contextMnu.addAction( configurefriendAct);
      contextMnu.addSeparator();
      contextMnu.addAction( connectfriendAct); 
      contextMnu.addAction( exportfriendAct);
      contextMnu.addAction( removefriendAct);
      contextMnu.exec( mevent->globalPos() );
}



/* get the list of peers from the RsIface.  */
void  PeersDialog::insertPeers()
{
	std::list<std::string> peers;
	std::list<std::string>::iterator it;

	if (!rsPeers)
        {
                /* not ready yet! */
                return;
        }

	rsPeers->getFriendList(peers);

        /* get a link to the table */
        QTreeWidget *peerWidget = ui.peertreeWidget;
        QTreeWidgetItem *oldSelect = getCurrentPeer();
        QTreeWidgetItem *newSelect = NULL;
	time_t now = time(NULL);

        std::string oldId;
        if (oldSelect)
        {
                oldId = (oldSelect -> text(7)).toStdString();
        }

        /* remove old items ??? */
	peerWidget->clear();
	peerWidget->setColumnCount(8);
	

        QList<QTreeWidgetItem *> items;
	for(it = peers.begin(); it != peers.end(); it++)
	{
		RsPeerDetails detail;
		if (!rsPeers->getPeerDetails(*it, detail))
		{
			continue; /* BAD */
		}
		
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* First 5 (1-5) Key Items */
		/* () Status Icon */
		item -> setText(0, "");
		
		/* (0) Status */		
		QString status = QString::fromStdString(RsPeerStateString(detail.state));

		/* Append additional status info from status service */
		StatusInfo statusInfo;
		if ((rsStatus) && (rsStatus->getStatus(*it, statusInfo)))
		{
			status.append(QString::fromStdString("/" + RsStatusString(statusInfo.status)));
		}

		item -> setText(1, status);

		/* (1) Person */
		item -> setText(2, QString::fromStdString(detail.name));

		/* (2) Auto Connect */
		item -> setText(3, QString::fromStdString(detail.autoconnect));

		/* (3) Trust Level */
                item -> setText(4,QString::fromStdString(
				RsPeerTrustString(detail.trustLvl)));
		
		/* (4) Peer Address */
		{
			std::ostringstream out;
			out << detail.localAddr << ":";
			out << detail.localPort << "/";
			out << detail.extAddr << ":";
			out << detail.extPort;
			item -> setText(5, QString::fromStdString(out.str()));
		}
		
		/* less important ones */
		/* () Last Contact */
                item -> setText(6,QString::fromStdString(
				RsPeerLastConnectString(now - detail.lastConnect)));

		/* () Org */
		//item -> setText(7, QString::fromStdString(detail.org));
		/* () Location */
		//item -> setText(8, QString::fromStdString(detail.location));
		/* () Email */
		//item -> setText(9, QString::fromStdString(detail.email));
	
		/* Hidden ones: RsCertId */
		{
			item -> setText(7, QString::fromStdString(detail.id));
                        if ((oldSelect) && (oldId == detail.id))
                        {
                                newSelect = item;
                        }
		}

		/* ()  AuthCode */	
        //        item -> setText(11, QString::fromStdString(detail.authcode));

		/* change background */
		int i;
		if (detail.state & RS_PEER_STATE_CONNECTED)
		{
			/* bright green */
			for(i = 1; i < 8; i++)
			{
			  item -> setBackground(i,QBrush(Qt::green));
			  item -> setIcon(0,(QIcon(IMAGE_ONLINE)));
			}
		}
		else if (detail.state & RS_PEER_STATE_UNREACHABLE)
		{
			/* bright green */
			for(i = 1; i < 8; i++)
			{
			  item -> setBackground(i,QBrush(Qt::red));
			  item -> setIcon(0,(QIcon(IMAGE_OFFLINE)));
			}
		}
		else if (detail.state & RS_PEER_STATE_ONLINE)
		{
			/* bright green */
			for(i = 1; i < 8; i++)
			{
			  item -> setBackground(i,QBrush(Qt::cyan));
			  item -> setIcon(0,(QIcon(IMAGE_OFFLINE)));
			}
		}
		else 
		{
                	if (now - detail.lastConnect < 3600)
			{
				for(i = 1; i < 8; i++)
				{
				  item -> setBackground(i,QBrush(Qt::lightGray));
				  item -> setIcon(0,(QIcon(IMAGE_OFFLINE)));
				}
			}
			else
			{
				for(i = 1; i < 8; i++)
				{
				  item -> setBackground(i,QBrush(Qt::gray));
				  item -> setIcon(0,(QIcon(IMAGE_OFFLINE)));
				}
			}
		}
			
		/* add to the list */
		items.append(item);
	}

	/* add the items in! */
	peerWidget->insertTopLevelItems(0, items);
        if (newSelect)
        {
                peerWidget->setCurrentItem(newSelect);
        }

	peerWidget->update(); /* update display */
}

/* Utility Fns */
std::string getPeerRsCertId(QTreeWidgetItem *i)
{
	std::string id = (i -> text(7)).toStdString();
	return id;
}

/** Open a QFileDialog to browse for export a file. */
void PeersDialog::exportfriend()
{
        QTreeWidgetItem *c = getCurrentPeer();

#ifdef PEERS_DEBUG 
        std::cerr << "PeersDialog::exportfriend()" << std::endl;
#endif
	if (!c)
	{
#ifdef PEERS_DEBUG 
        	std::cerr << "PeersDialog::exportfriend() Noone Selected -- sorry" << std::endl;
#endif
		return;
	}

	std::string id = getPeerRsCertId(c);
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Certificate"), "",
	                                                     tr("Certificates (*.pqi)"));

	std::string file = fileName.toStdString();
	if (file != "")
	{
#ifdef PEERS_DEBUG 
        	std::cerr << "PeersDialog::exportfriend() Saving to: " << file << std::endl;
        	std::cerr << std::endl;
#endif
		if (rsPeers)
		{
			rsPeers->SaveCertificateToFile(id, file);
		}
	}

}

void PeersDialog::chatfriend()
{
    QTreeWidgetItem *i = getCurrentPeer();

    if (!i)
	return;

    std::string name = (i -> text(2)).toStdString();
    std::string id = (i -> text(7)).toStdString();
    
    RsPeerDetails detail;
    if (!rsPeers->getPeerDetails(id, detail))
    {
    	return;
    }

    if (detail.state & RS_PEER_STATE_CONNECTED)
    {
    	/* must reference ChatDialog */
    	if (chatDialog)
    	{
    		chatDialog->getPrivateChat(id, name, true);
    	}
    }
    else
    {
    	/* info dialog */
        QMessageBox::StandardButton sb = QMessageBox::question ( NULL, 
			"Friend Not Online", 
	"Your Friend is offline \nDo you want to send them a Message instead",
			        (QMessageBox::Yes | QMessageBox::No));
	if (sb == QMessageBox::Yes)
	{
		msgfriend();
	}
    }
    return;
}


void PeersDialog::msgfriend()
{
#ifdef PEERS_DEBUG 
    std::cerr << "SharedFilesDialog::msgfriend()" << std::endl;
#endif

    QTreeWidgetItem *i = getCurrentPeer();

    if (!i)
	return;

    std::string status = (i -> text(1)).toStdString();
    std::string name = (i -> text(2)).toStdString();
    std::string id = (i -> text(7)).toStdString();

    rsicontrol -> ClearInMsg();
    rsicontrol -> SetInMsg(id, true);

    /* create a message */
    ChanMsgDialog *nMsgDialog = new ChanMsgDialog(true);

    nMsgDialog->newMsg();
    nMsgDialog->show();
}


QTreeWidgetItem *PeersDialog::getCurrentPeer()
{
	/* get the current, and extract the Id */

	/* get a link to the table */
        QTreeWidget *peerWidget = ui.peertreeWidget;
        QTreeWidgetItem *item = peerWidget -> currentItem();
        if (!item)
        {
#ifdef PEERS_DEBUG 
		std::cerr << "Invalid Current Item" << std::endl;
#endif
		return NULL;
	}

#ifdef PEERS_DEBUG 
	/* Display the columns of this item. */
	std::ostringstream out;
        out << "CurrentPeerItem: " << std::endl;

	for(int i = 1; i < 6; i++)
	{
		QString txt = item -> text(i);
		out << "\t" << i << ":" << txt.toStdString() << std::endl;
	}
	std::cerr << out.str();
#endif
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
 

void PeersDialog::removefriend()
{
        QTreeWidgetItem *c = getCurrentPeer();
#ifdef PEERS_DEBUG 
        std::cerr << "PeersDialog::removefriend()" << std::endl;
#endif
	if (!c)
	{
#ifdef PEERS_DEBUG 
        	std::cerr << "PeersDialog::removefriend() Noone Selected -- sorry" << std::endl;
#endif
		return;
	}

	if (rsPeers)
	{
		rsPeers->removeFriend(getPeerRsCertId(c));
	}
}


void PeersDialog::allowfriend()
{
	QTreeWidgetItem *c = getCurrentPeer();
#ifdef PEERS_DEBUG 
	std::cerr << "PeersDialog::allowfriend()" << std::endl;
#endif
	/*
	bool accept = true;
	rsServer->FriendStatus(getPeerRsCertId(c), accept);
	*/
}


void PeersDialog::connectfriend()
{
	QTreeWidgetItem *c = getCurrentPeer();
#ifdef PEERS_DEBUG 
	std::cerr << "PeersDialog::connectfriend()" << std::endl;
#endif
	if (!c)
	{
#ifdef PEERS_DEBUG 
        	std::cerr << "PeersDialog::connectfriend() Noone Selected -- sorry" << std::endl;
#endif
		return;
	}

	if (rsPeers)
	{
		rsPeers->connectAttempt(getPeerRsCertId(c));
	}
}

void PeersDialog::setaddressfriend()
{
	QTreeWidgetItem *c = getCurrentPeer();
#ifdef PEERS_DEBUG 
	std::cerr << "PeersDialog::setaddressfriend()" << std::endl;
#endif

	/* need to get the input address / port */
	/*
 	std::string addr;
	unsigned short port;
	rsServer->FriendSetAddress(getPeerRsCertId(c), addr, port);
	*/
}

void PeersDialog::trustfriend()
{
	QTreeWidgetItem *c = getCurrentPeer();
#ifdef PEERS_DEBUG 
	std::cerr << "PeersDialog::trustfriend()" << std::endl;
#endif
	/*
	bool trust = true;
	rsServer->FriendTrust(getPeerRsCertId(c), trust);
	*/
}



/* GUI stuff -> don't do anything directly with Control */
void PeersDialog::configurefriend()
{
	/* display Dialog */
#ifdef PEERS_DEBUG 
	std::cerr << "PeersDialog::configurefriend()" << std::endl;
#endif
	QTreeWidgetItem *c = getCurrentPeer();


	static ConfCertDialog *confdialog = new ConfCertDialog();


	if (!c)
		return;

	/* set the Id */
	std::string id = getPeerRsCertId(c);

	confdialog -> loadId(id);
	confdialog -> show();
}



