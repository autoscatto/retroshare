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
#include "rsiface/rsmsgs.h"
#include "rsiface/rsnotify.h"

#include "chat/PopupChatDialog.h"
#include "msgs/ChanMsgDialog.h"
#include "connect/ConfCertDialog.h"
#include "profile/ProfileView.h"
#include "profile/ProfileWidget.h"
#include "profile/StatusMessage.h"

#include "GenCertDialog.h"
#include "gui/connect/ConnectFriendWizard.h"

#include <sstream>

#include <QTextCodec>
#include <QTextEdit>
#include <QTextCursor>
#include <QTextList>
#include <QTextStream>
#include <QTextDocumentFragment>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QMessageBox>
#include <QHeaderView>
#include <QtGui/QKeyEvent>


/* Images for context menu icons */
#define IMAGE_REMOVEFRIEND       ":/images/removefriend16.png"
#define IMAGE_EXPIORTFRIEND      ":/images/exportpeers_16x16.png"
#define IMAGE_PEERINFO           ":/images/peerdetails_16x16.png"
#define IMAGE_CHAT               ":/images/chat.png"
#define IMAGE_MSG                ":/images/message-mail.png"
#define IMAGE_CONNECT            ":/images/connect_friend.png"
/* Images for Status icons */
#define IMAGE_ONLINE             ":/images/user/identity24.png"
#define IMAGE_OFFLINE            ":/images/user/identityoffline24.png"
#define IMAGE_OFFLINE2           ":/images/user/identitylightgrey24.png"
#define IMAGE_AVAIBLE            ":/images/user/identityavaiblecyan24.png"
#define IMAGE_UNREACHABLE        ":/images/user/identityunreachable24.png"
#define IMAGE_CONNECT2           ":/images/reload24.png"


/******
 * #define PEERS_DEBUG 1
 *****/


/** Constructor */
PeersDialog::PeersDialog(QWidget *parent)
            : RsAutoUpdatePage(1500,parent),
              historyKeeper(Rshare::dataDirectory() + "/his1.xml")
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);
  
  /* Create RshareSettings object */
  _settings = new RshareSettings();

  last_status_send_time = 0 ;

  connect( ui.peertreeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( peertreeWidgetCostumPopupMenu( QPoint ) ) );
  connect( ui.peertreeWidget, SIGNAL( itemDoubleClicked ( QTreeWidgetItem *, int)), this, SLOT(chatfriend()));

  connect( ui.avatartoolButton, SIGNAL(clicked()), SLOT(getAvatar()));
  connect( ui.mypersonalstatuslabel, SIGNAL(clicked()), SLOT(statusmessage()));

  /* hide the Tree +/- */
  ui.peertreeWidget -> setRootIsDecorated( false );
  
  ui.peertabWidget->addTab(new ProfileWidget(),QString(tr("Profile")));

  ui.peertreeWidget->setColumnCount(8);
  ui.peertreeWidget->setColumnHidden ( 3, true);
  ui.peertreeWidget->setColumnHidden ( 4, true);
  ui.peertreeWidget->setColumnHidden ( 5, true);
  ui.peertreeWidget->setColumnHidden ( 6, true);
  ui.peertreeWidget->setColumnHidden ( 7, true);
  ui.peertreeWidget->sortItems( 2, Qt::AscendingOrder );

  /* Set header resize modes and initial section sizes */
	QHeaderView * _header = ui.peertreeWidget->header () ;
        _header->setResizeMode (0, QHeaderView::Custom);
	_header->setResizeMode (1, QHeaderView::Interactive);
	_header->setResizeMode (2, QHeaderView::Interactive);


	_header->resizeSection ( 0, 25 );
        _header->resizeSection ( 1, 150 );
        _header->resizeSection ( 2, 150 );


    // set header text aligment
	QTreeWidgetItem * headerItem = ui.peertreeWidget->headerItem();
	headerItem->setTextAlignment(0, Qt::AlignHCenter | Qt::AlignVCenter);
	headerItem->setTextAlignment(1, Qt::AlignHCenter | Qt::AlignVCenter);
	headerItem->setTextAlignment(2, Qt::AlignHCenter | Qt::AlignVCenter);



  loadEmoticonsgroupchat();


  connect(ui.lineEdit, SIGNAL(textChanged ( ) ), this, SLOT(checkChat( ) ));
  connect(ui.Sendbtn, SIGNAL(clicked()), this, SLOT(sendMsg()));
  connect(ui.emoticonBtn, SIGNAL(clicked()), this, SLOT(smileyWidgetgroupchat()));


  //connect( ui.msgSendList, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( msgSendListCostumPopupMenu( QPoint ) ) );
  connect( ui.msgText, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(displayInfoChatMenu(const QPoint&)));

  connect(ui.textboldChatButton, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(ui.textunderlineChatButton, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(ui.textitalicChatButton, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(ui.fontsButton, SIGNAL(clicked()), this, SLOT(getFont()));
  connect(ui.colorChatButton, SIGNAL(clicked()), this, SLOT(setColor()));

  ui.fontsButton->setIcon(QIcon(QString(":/images/fonts.png")));

  _currentColor = Qt::black;
  QPixmap pxm(16,16);
  pxm.fill(_currentColor);
  ui.colorChatButton->setIcon(pxm);

  mCurrentFont = QFont("Comic Sans MS", 12);
  ui.lineEdit->setFont(mCurrentFont);

  setChatInfo(tr("Welcome to RetroShare's group chat."),
              QString::fromUtf8("blue"));

  QStringList him;
  historyKeeper.getMessages(him, "", "THIS", 8);
  foreach(QString mess, him)
      ui.msgText->append(mess);
      //setChatInfo(mess,  "green");


  QMenu * grpchatmenu = new QMenu();
  grpchatmenu->addAction(ui.actionClearChat);
  ui.menuButton->setMenu(grpchatmenu);

  _underline = false;

  QTimer *timer = new QTimer(this);
  timer->connect(timer, SIGNAL(timeout()), this, SLOT(insertChat()));
  timer->start(500); /* half a second */
  
  QMenu *menu = new QMenu();
  menu->addAction(ui.actionAdd_Friend); 
  menu->addSeparator();
  menu->addAction(ui.actionCreate_new_Profile);
  ui.menupushButton->setMenu(menu);

  
  updateAvatar();
  loadmypersonalstatus();

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void PeersDialog::peertreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      chatAct = new QAction(QIcon(IMAGE_CHAT), tr( "Chat" ), this );
      connect( chatAct , SIGNAL( triggered() ), this, SLOT( chatfriend() ) );

      msgAct = new QAction(QIcon(IMAGE_MSG), tr( "Message Friend" ), this );
      connect( msgAct , SIGNAL( triggered() ), this, SLOT( msgfriend() ) );

      connectfriendAct = new QAction(QIcon(IMAGE_CONNECT), tr( "Connect To Friend" ), this );
      connect( connectfriendAct , SIGNAL( triggered() ), this, SLOT( connectfriend() ) );

      configurefriendAct = new QAction(QIcon(IMAGE_PEERINFO), tr( "Peer Details" ), this );
      connect( configurefriendAct , SIGNAL( triggered() ), this, SLOT( configurefriend() ) );

      profileviewAct = new QAction(QIcon(IMAGE_PEERINFO), tr( "Profile View" ), this );
      connect( profileviewAct , SIGNAL( triggered() ), this, SLOT( viewprofile() ) );

      exportfriendAct = new QAction(QIcon(IMAGE_EXPIORTFRIEND), tr( "Export Friend" ), this );
      connect( exportfriendAct , SIGNAL( triggered() ), this, SLOT( exportfriend() ) );

      removefriendAct = new QAction(QIcon(IMAGE_REMOVEFRIEND), tr( "Deny Friend" ), this );
      connect( removefriendAct , SIGNAL( triggered() ), this, SLOT( removefriend() ) );


      QWidget *widget = new QWidget();
      widget->setStyleSheet( ".QWidget{background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 #FEFEFE, stop:1 #E8E8E8); border: 1px solid #CCCCCC;}");  
      
      QHBoxLayout *hbox = new QHBoxLayout();
      hbox->setMargin(0);
      hbox->setSpacing(6);
    
      iconLabel = new QLabel( this );
      iconLabel->setPixmap(QPixmap::QPixmap(":/images/user/friends24.png"));
      iconLabel->setMaximumSize( iconLabel->frameSize().height() + 24, 24 );
      hbox->addWidget(iconLabel);
       
      textLabel = new QLabel( tr("<strong>Friends</strong>"), this );
      hbox->addWidget(textLabel);
      
      spacerItem = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
      hbox->addItem(spacerItem); 
       
      widget->setLayout( hbox );
    
      QWidgetAction *widgetAction = new QWidgetAction(this); 
      widgetAction->setDefaultWidget(widget); 

      contextMnu.clear();
      contextMnu.addAction( widgetAction);
      contextMnu.addAction( chatAct);
      contextMnu.addAction( msgAct);
      contextMnu.addSeparator();
      contextMnu.addAction( configurefriendAct);
      //contextMnu.addAction( profileviewAct);
      contextMnu.addSeparator();
      contextMnu.addAction( connectfriendAct);
      contextMnu.addAction( exportfriendAct);
      contextMnu.addAction( removefriendAct);
      contextMnu.exec( mevent->globalPos() );
      

}

void PeersDialog::updateDisplay()
{
	insertPeers() ;
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

	// add self nick and Avatar to Friends.
	RsPeerDetails pd ;

        if (rsPeers->getPeerDetails(rsPeers->getOwnId(),pd))
        {
                QString titleStr("<span style=\"font-size:16pt; font-weight:500;"
                       "color:#32cd32;\">%1</span>");
                ui.nicklabel->setText(titleStr.arg(QString::fromStdString(pd.name) + tr(" (me)"))) ;
        }


	for(it = peers.begin(); it != peers.end(); it++)
	{
		RsPeerDetails detail;
		if (!rsPeers->getPeerDetails(*it, detail))
		{
			continue; /* BAD */
		}

		/* make a widget per friend */
                QTreeWidgetItem *item;
                QList<QTreeWidgetItem *> list = peerWidget->findItems (QString::fromStdString(detail.id), Qt::MatchExactly, 7);
                if (list.size() == 1) {
                    item = list.front();
                } else {
                    item = new QTreeWidgetItem((QTreeWidget*)0);
                }

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

		//item -> setText(1, status);
		item -> setText(1, QString::fromStdString(detail.autoconnect));
		item -> setTextAlignment(1, Qt::AlignCenter | Qt::AlignVCenter );

		/* (1) Person */
                if (rsMsgs->getCustomStateString(detail.id) != "") {
                    item -> setText( 2, QString::fromStdString(detail.name) + tr(" - ") +
                    QString::fromStdString(rsMsgs->getCustomStateString(detail.id)));
                    item -> setToolTip( 2, QString::fromStdString(detail.name) + tr(" - ") +
                    QString::fromStdString(rsMsgs->getCustomStateString(detail.id)));
                } else {
                    item -> setText( 2, QString::fromStdString(detail.name));
                    item -> setToolTip( 2, QString::fromStdString(detail.name));
                }

                item -> setText(3, status);

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
                                peerWidget->setCurrentItem(item);
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
				// CsoLer: I uncommented the color because it's really confortable
				// to be able to see at some distance that people are connected.
				// The blue/gray icons need a close look indeed.
			  //item -> setBackground(i,QBrush(Qt::green))
			  item -> setTextColor(i,(Qt::darkBlue));
			  QFont font ;
                          font.setBold(true);
			  item -> setFont(i,font);
			  item -> setIcon(0,(QIcon(IMAGE_ONLINE)));
			}
		}
		else if (detail.state & RS_PEER_STATE_UNREACHABLE)
		{
			/* bright green */
			for(i = 1; i < 8; i++)
			{
			  //item -> setBackground(i,QBrush(Qt::red));
			  item -> setTextColor(i,(Qt::darkRed));
			  QFont font ;
			  item -> setFont(i,font);
			  item -> setIcon(0,(QIcon(IMAGE_UNREACHABLE)));
			}
		}
		else if (detail.state & RS_PEER_STATE_ONLINE)
		{
			/* bright green */
			for(i = 1; i < 8; i++)
			{
			  //item -> setBackground(i,QBrush(Qt::cyan));
			  item -> setTextColor(i,(Qt::darkCyan));
			  QFont font ;
                          font.setBold(true);
			  item -> setFont(i,font);
			  item -> setIcon(0,(QIcon(IMAGE_AVAIBLE)));
			}
		}
		else
		{
                	if (now - detail.lastConnect < 3600)
			{
				for(i = 1; i < 8; i++)
				{
				  //item -> setBackground(i,QBrush(Qt::lightGray));
				  item -> setIcon(0,(QIcon(IMAGE_OFFLINE)));
				}
			}
			else
			{
				for(i = 1; i < 8; i++)
				{
				  //item -> setBackground(i,QBrush(Qt::gray));
				  item -> setIcon(0,(QIcon(IMAGE_OFFLINE2)));
				}
			}
		}

		/* add to the list */
                peerWidget->addTopLevelItem(item);
	}
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
    	getPrivateChat(id, name, RS_CHAT_REOPEN);
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
		emit friendsUpdated() ;
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
		c -> setIcon(0,(QIcon(IMAGE_CONNECT2)));
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
	ConfCertDialog::show(getPeerRsCertId(getCurrentPeer()));
}

void PeersDialog::resetStatusBar() 
{
	std::cerr << "PeersDialog: reseting status bar." << std::endl ;

	ui.statusStringLabel->setText(QString("")) ;
}

void PeersDialog::updateStatusTyping()
{
	if(time(NULL) - last_status_send_time > 5)	// limit 'peer is typing' packets to at most every 10 sec
	{
		std::cerr << "PeersDialog: sending group chat typing info." << std::endl ;

		rsMsgs->sendGroupChatStatusString(rsiface->getConfig().ownName + " is typing...");
		last_status_send_time = time(NULL) ;
	}
}
// Called by libretroshare through notifyQt to display the peer's status
//
void PeersDialog::updateStatusString(const QString& status_string)
{
	std::cerr << "PeersDialog: received group chat typing info. updating gui." << std::endl ;

	ui.statusStringLabel->setText(status_string) ; // displays info for 5 secs.

	QTimer::singleShot(5000,this,SLOT(resetStatusBar())) ;
}

void PeersDialog::updatePeersCustomStateString(const QString& peer_id)
{
#ifdef JUST_AN_EXAMPLE
	// This is an example of how to retrieve the custom string.
	//
	std::cerr << "PeersDialog: Got notified that state string changed for peer " << peer_id.toStdString() << std::endl ;
	std::cerr << "New state string for this peer is : " << rsMsgs->getCustomStateString(peer_id.toStdString()) << std::endl ;

	QMessageBox::information(NULL,"Notification",peer_id+" has new custom string: " + QString::fromStdString(rsMsgs->getCustomStateString(peer_id.toStdString()))) ;
#endif
}

void PeersDialog::updatePeersAvatar(const QString& peer_id)
{
	std::cerr << "PeersDialog: Got notified of new avatar for peer " << peer_id.toStdString() << std::endl ;

	PopupChatDialog *pcd = getPrivateChat(peer_id.toStdString(),rsPeers->getPeerName(peer_id.toStdString()), 0);
	pcd->updatePeerAvatar(peer_id.toStdString());
}

void PeersDialog::updatePeerStatusString(const QString& peer_id,const QString& status_string,bool is_private_chat)
{
	if(is_private_chat)
	{
		PopupChatDialog *pcd = getPrivateChat(peer_id.toStdString(),rsPeers->getPeerName(peer_id.toStdString()), 0);
		pcd->updateStatusString(status_string);
	}
	else
	{
		std::cerr << "Updating public chat msg from peer " << rsPeers->getPeerName(peer_id.toStdString()) << ": " << status_string.toStdString() << std::endl ;

		updateStatusString(status_string) ;
	}
}

void PeersDialog::insertChat()
{
	if (!rsMsgs->chatAvailable())
	{
#ifdef PEERS_DEBUG
		std::cerr << "no chat available." << std::endl ;
#endif
		return;
	}

	std::list<ChatInfo> newchat;
	if (!rsMsgs->getNewChat(newchat))
	{
		std::cerr << "could not get new chat." << std::endl ;
		return;
	}

		std::cerr << "got new chat." << std::endl ;
    QTextEdit *msgWidget = ui.msgText;
	std::list<ChatInfo>::iterator it;

        /** A RshareSettings object used for saving/loading settings */
        RshareSettings settings;
        uint chatflags = settings.getChatFlags();

	/* add in lines at the bottom */
	for(it = newchat.begin(); it != newchat.end(); it++)
	{
		std::string msg(it->msg.begin(), it->msg.end());
#ifdef PEERS_DEBUG
		std::cerr << "PeersDialog::insertChat(): " << msg << std::endl;
#endif

		/* are they private? */
		if (it->chatflags & RS_CHAT_PRIVATE)
		{
			PopupChatDialog *pcd = getPrivateChat(it->rsid, it->name, chatflags);
			pcd->addChatMsg(&(*it));
			QApplication::alert(pcd);
			continue;
		}

		std::ostringstream out;
		QString extraTxt;

        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString name = QString::fromStdString(it->name);
        QString line = "<span style=\"color:#C00000\">" + timestamp + "</span>" +
            		"<span style=\"color:#2D84C9\"><strong>" + " " + name + "</strong></span>";

        //std::cerr << "PeersDialog::insertChat(): 1.11\n";
        historyKeeper.addMessage(name, "THIS", QString::fromStdWString(it->msg));
        //std::cerr << "PeersDialog::insertChat(): 1.12\n";
        extraTxt += line;

        extraTxt += QString::fromStdWString(it->msg);

		  // notify with a systray icon msg
		  if(it->rsid != rsPeers->getOwnId())
		  {
			  // This is a trick to translate HTML into text.
			  QTextEdit editor ;
			  editor.setHtml(QString::fromStdWString(it->msg));
			  QString notifyMsg(QString::fromStdString(it->name)+": "+editor.toPlainText()) ;

			  if(notifyMsg.length() > 30)
				  emit notifyGroupChat(QString("New group chat"), notifyMsg.left(30)+QString("..."));
			  else
				  emit notifyGroupChat(QString("New group chat"), notifyMsg);
		  }

		QHashIterator<QString, QString> i(smileys);
		while(i.hasNext())
		{
			i.next();
			foreach(QString code, i.key().split("|"))
				extraTxt.replace(code, "<img src=\"" + i.value() + "\" />");
		}

		if ((msgWidget->verticalScrollBar()->maximum() - 30) < msgWidget->verticalScrollBar()->value() ) {
		    msgWidget->append(extraTxt);
		} else {
		    //the vertical scroll is not at the bottom, so just update the text, the scroll will stay at the current position
		    int scroll = msgWidget->verticalScrollBar()->value();
		    msgWidget->setHtml(msgWidget->toHtml() + extraTxt);
		    msgWidget->verticalScrollBar()->setValue(scroll);
		    msgWidget->update();
		}
	}
}

void PeersDialog::checkChat()
{
	/* if <return> at the end of the text -> we can send it! */
        QTextEdit *chatWidget = ui.lineEdit;
        std::string txt = chatWidget->toPlainText().toStdString();
	if ('\n' == txt[txt.length()-1])
	{
		//std::cerr << "Found <return> found at end of :" << txt << ": should send!";
		//std::cerr << std::endl;
		if (txt.length()-1 == txt.find('\n')) /* only if on first line! */
		{
			/* should remove last char ... */
			sendMsg();
		}
	}
	else
	{
		updateStatusTyping() ;

		//std::cerr << "No <return> found in :" << txt << ":";
		//std::cerr << std::endl;
	}
}

void PeersDialog::sendMsg()
{
    QTextEdit *lineWidget = ui.lineEdit;

	ChatInfo ci;
	//ci.msg = lineWidget->Text().toStdWString();
	ci.msg = lineWidget->toHtml().toStdWString();
	ci.chatflags = RS_CHAT_PUBLIC;

    //historyKeeper.addMessage("THIS", "ALL", lineWidget->toHtml() );

	std::string msg(ci.msg.begin(), ci.msg.end());
#ifdef PEERS_DEBUG
	std::cerr << "PeersDialog::sendMsg(): " << msg << std::endl;
#endif

	rsMsgs -> ChatSend(ci);
	ui.lineEdit->clear();
	setFont();

	/* redraw send list */
	insertSendList();

}

void  PeersDialog::insertSendList()
{
	std::list<std::string> peers;
	std::list<std::string>::iterator it;

	if (!rsPeers)
	{
		/* not ready yet! */
		return;
	}

	rsPeers->getOnlineList(peers);

        /* get a link to the table */
        //QTreeWidget *sendWidget = ui.msgSendList;
	QList<QTreeWidgetItem *> items;

	for(it = peers.begin(); it != peers.end(); it++)
	{

		RsPeerDetails details;
		if (!rsPeers->getPeerDetails(*it, details))
		{
			continue; /* BAD */
		}

		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		/* (0) Person */
		item -> setText(0, QString::fromStdString(details.name));

		item -> setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
		//item -> setFlags(Qt::ItemIsUserCheckable);

		item -> setCheckState(0, Qt::Checked);

		if (rsicontrol->IsInChat(*it))
		{
			item -> setCheckState(0, Qt::Checked);
		}
		else
		{
			item -> setCheckState(0, Qt::Unchecked);
		}

		/* disable for the moment */
		item -> setFlags(Qt::ItemIsUserCheckable);
		item -> setCheckState(0, Qt::Checked);

		/* add to the list */
		items.append(item);
	}

        /* remove old items */
	//sendWidget->clear();
	//sendWidget->setColumnCount(1);

	/* add the items in! */
	//sendWidget->insertTopLevelItems(0, items);

	//sendWidget->update(); /* update display */
}


/* to toggle the state */


void PeersDialog::toggleSendItem( QTreeWidgetItem *item, int col )
{
#ifdef PEERS_DEBUG
	std::cerr << "ToggleSendItem()" << std::endl;
#endif

	/* extract id */
	std::string id = (item -> text(4)).toStdString();

	/* get state */
	bool inChat = (Qt::Checked == item -> checkState(0)); /* alway column 0 */

	/* call control fns */

	rsicontrol -> SetInChat(id, inChat);
	return;
}

//============================================================================

PopupChatDialog *
PeersDialog::getPrivateChat(std::string id, std::string name, uint chatflags)
{
   /* see if it exists already */
   PopupChatDialog *popupchatdialog = NULL;
   bool show = false;

   if (chatflags & RS_CHAT_REOPEN)
   {
  	show = true;
	std::cerr << "reopen flag so: enable SHOW popupchatdialog()";
	std::cerr << std::endl;
   }


   std::map<std::string, PopupChatDialog *>::iterator it;
   if (chatDialogs.end() != (it = chatDialogs.find(id)))
   {
   	/* exists already */
   	popupchatdialog = it->second;
   }
   else
   {
   	popupchatdialog = new PopupChatDialog(id, name);
	chatDialogs[id] = popupchatdialog;

	if (chatflags & RS_CHAT_OPEN_NEW)
	{
		std::cerr << "new chat so: enable SHOW popupchatdialog()";
		std::cerr << std::endl;

		show = true;
	}
   }

   if (show)
   {
	std::cerr << "SHOWING popupchatdialog()";
	std::cerr << std::endl;

	popupchatdialog->show();
   }

   /* now only do these if the window is visible */
   if (popupchatdialog->isVisible())
   {
	   if (chatflags & RS_CHAT_FOCUS)
	   {
		std::cerr << "focus chat flag so: GETFOCUS popupchatdialog()";
		std::cerr << std::endl;

		popupchatdialog->getfocus();
	   }
	   else
	   {
		std::cerr << "no focus chat flag so: FLASH popupchatdialog()";
		std::cerr << std::endl;

		popupchatdialog->flash();
	   }
   }
   else
   {
	std::cerr << "not visible ... so leave popupchatdialog()";
	std::cerr << std::endl;
   }

   return popupchatdialog;
}

//============================================================================

void PeersDialog::clearOldChats()
{
	/* nothing yet */

}

void PeersDialog::setColor()
{

    	bool ok;
 	QRgb color = QColorDialog::getRgba(ui.lineEdit->textColor().rgba(), &ok, this);
 	if (ok) {
 	        _currentColor = QColor(color);
 	        QPixmap pxm(16,16);
	        pxm.fill(_currentColor);
	        ui.colorChatButton->setIcon(pxm);
 	}
	setFont();
}

void PeersDialog::getFont()
{
    bool ok;
    mCurrentFont = QFontDialog::getFont(&ok, mCurrentFont, this);
    setFont();
}

void PeersDialog::setFont()
{
  mCurrentFont.setBold(ui.textboldChatButton->isChecked());
  mCurrentFont.setUnderline(ui.textunderlineChatButton->isChecked());
  mCurrentFont.setItalic(ui.textitalicChatButton->isChecked());
  ui.lineEdit->setFont(mCurrentFont);
  ui.lineEdit->setTextColor(_currentColor);

  ui.lineEdit->setFocus();

}

void PeersDialog::underline()
{
 	        _underline = !_underline;
 	        ui.lineEdit->setFontUnderline(_underline);
}


// Update Chat Info information
void PeersDialog::setChatInfo(QString info, QColor color)
{
  static unsigned int nbLines = 0;
  ++nbLines;
  // Check log size, clear it if too big
  if(nbLines > 200) {
    ui.msgText->clear();
    nbLines = 1;
  }
  ui.msgText->append(QString::fromUtf8("<font color='grey'>")+ QTime::currentTime().toString(QString::fromUtf8("hh:mm:ss")) + QString::fromUtf8("</font> - <font color='") + color.name() +QString::fromUtf8("'><i>") + info + QString::fromUtf8("</i></font>"));
}

void PeersDialog::on_actionClearChat_triggered()
{
  ui.msgText->clear();
}

void PeersDialog::displayInfoChatMenu(const QPoint& pos)
{
  // Log Menu
  QMenu myChatMenu(this);
  myChatMenu.addAction(ui.actionClearChat);
  // XXX: Why mapToGlobal() is not enough?
  myChatMenu.exec(mapToGlobal(pos)+QPoint(0,80));
}

void PeersDialog::loadEmoticonsgroupchat()
{
	QString sm_codes;
	#if defined(Q_OS_WIN32)
	QFile sm_file(QApplication::applicationDirPath() + "/emoticons/emotes.acs");
	#else
	QFile sm_file(QString(":/smileys/emotes.acs"));
	#endif
	if(!sm_file.open(QIODevice::ReadOnly))
	{
		std::cerr << "Could not open resouce file :/emoticons/emotes.acs" << endl ;
		return ;
	}
	sm_codes = sm_file.readAll();
	sm_file.close();
	sm_codes.remove("\n");
	sm_codes.remove("\r");
	int i = 0;
	QString smcode;
	QString smfile;
	while(sm_codes[i] != '{')
	{
		i++;

	}
	while (i < sm_codes.length()-2)
	{
		smcode = "";
		smfile = "";
		while(sm_codes[i] != '\"')
		{
			i++;
		}
		i++;
		while (sm_codes[i] != '\"')
		{
			smcode += sm_codes[i];
			i++;

		}
		i++;

		while(sm_codes[i] != '\"')
		{
			i++;
		}
		i++;
		while(sm_codes[i] != '\"' && sm_codes[i+1] != ';')
		{
			smfile += sm_codes[i];
			i++;
		}
		i++;
		if(!smcode.isEmpty() && !smfile.isEmpty())
			#if defined(Q_OS_WIN32)
		    smileys.insert(smcode, smfile);
	        #else
			smileys.insert(smcode, ":/"+smfile);
			#endif
	}
}

void PeersDialog::smileyWidgetgroupchat()
{
	qDebug("MainWindow::smileyWidget()");
	QWidget *smWidget = new QWidget(this , Qt::Popup );
	smWidget->setWindowTitle("Emoticons");
	smWidget->setWindowIcon(QIcon(QString(":/images/rstray3.png")));
	//smWidget->setFixedSize(256,256);

	smWidget->setBaseSize( 4*24, (smileys.size()/4)*24  );

    //Warning: this part of code was taken from kadu instant messenger;
    //         It was EmoticonSelector::alignTo(QWidget* w) function there
    //         comments are Polish, I dont' know how does it work...
    // oblicz pozycj� widgetu do kt�rego r�wnamy
    QWidget* w = ui.emoticonBtn;
    QPoint w_pos = w->mapToGlobal(QPoint(0,0));
    // oblicz rozmiar selektora
    QSize e_size = smWidget->sizeHint();
    // oblicz rozmiar pulpitu
    QSize s_size = QApplication::desktop()->size();
    // oblicz dystanse od widgetu do lewego brzegu i do prawego
    int l_dist = w_pos.x();
    int r_dist = s_size.width() - (w_pos.x() + w->width());
    // oblicz pozycj� w zale�no�ci od tego czy po lewej stronie
    // jest wi�cej miejsca czy po prawej
    int x;
    if (l_dist >= r_dist)
        x = w_pos.x() - e_size.width();
    else
        x = w_pos.x() + w->width();
    // oblicz pozycj� y - centrujemy w pionie
    int y = w_pos.y() + w->height()/2 - e_size.height()/2;
    // je�li wychodzi poza doln� kraw�d� to r�wnamy do niej
    if (y + e_size.height() > s_size.height())
        y = s_size.height() - e_size.height();
    // je�li wychodzi poza g�rn� kraw�d� to r�wnamy do niej
    if (y < 0)
         y = 0;
    // ustawiamy selektor na wyliczonej pozycji
    smWidget->move(x, y);

	x = 0;
    y = 0;

	QHashIterator<QString, QString> i(smileys);
	while(i.hasNext())
	{
		i.next();
		QPushButton *smButton = new QPushButton("", smWidget);
		smButton->setGeometry(x*24, y*24, 24,24);
		smButton->setIconSize(QSize(24,24));
		smButton->setIcon(QPixmap(i.value()));
		smButton->setToolTip(i.key());
		//smButton->setFixedSize(24,24);
		++x;
		if(x > 4)
		{
			x = 0;
			y++;
		}
		connect(smButton, SIGNAL(clicked()), this, SLOT(addSmileys()));
        connect(smButton, SIGNAL(clicked()), smWidget, SLOT(close()));
	}

	smWidget->show();
}

void PeersDialog::addSmileys()
{
	ui.lineEdit->setText(ui.lineEdit->toHtml() + qobject_cast<QPushButton*>(sender())->toolTip().split("|").first());
}

/* GUI stuff -> don't do anything directly with Control */
void PeersDialog::viewprofile()
{
	/* display Dialog */

	QTreeWidgetItem *c = getCurrentPeer();


	static ProfileView *profileview = new ProfileView();


	if (!c)
		return;

	/* set the Id */
	std::string id = getPeerRsCertId(c);

	profileview -> setPeerId(id);
	profileview -> show();
}

void PeersDialog::updateAvatar()
{
	unsigned char *data = NULL;
	int size = 0 ;

	rsMsgs->getOwnAvatarData(data,size); 

	std::cerr << "Image size = " << size << std::endl ;

	if(size == 0)
	   std::cerr << "Got no image" << std::endl ;

	// set the image
	QPixmap pix ;
	pix.loadFromData(data,size,"PNG") ;
	ui.avatartoolButton->setIcon(pix); // writes image into ba in PNG format

   for(std::map<std::string, PopupChatDialog *>::const_iterator it(chatDialogs.begin());it!=chatDialogs.end();++it)
		it->second->updateAvatar() ;

	delete[] data ;
}

void PeersDialog::getAvatar()
{
	QString fileName = QFileDialog::getOpenFileName(this, "Load File", QDir::homePath(), "Pictures (*.png *.xpm *.jpg *.tiff)");
	if(!fileName.isEmpty())
	{
		picture = QPixmap(fileName).scaled(82,82, Qt::IgnoreAspectRatio);

		std::cerr << "Sending avatar image down the pipe" << std::endl ;

		// send avatar down the pipe for other peers to get it.
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		picture.save(&buffer, "PNG"); // writes image into ba in PNG format

		std::cerr << "Image size = " << ba.size() << std::endl ;

		rsMsgs->setOwnAvatarData((unsigned char *)(ba.data()),ba.size()) ;	// last char 0 included.

		// I suppressed this because it gets called already by rsMsgs->setOwnAvatarData() through a Qt notification signal
		//updateAvatar() ;
	}
}

void PeersDialog::changeAvatarClicked() 
{

	updateAvatar();
}

void PeersDialog::on_actionAdd_Friend_activated() 
{
    ConnectFriendWizard* connectwiz = new ConnectFriendWizard(this);

    // set widget to be deleted after close
    connectwiz->setAttribute( Qt::WA_DeleteOnClose, true);


    connectwiz->show();
}

void PeersDialog::on_actionCreate_new_Profile_activated()
{
    static GenCertDialog *gencertdialog = new GenCertDialog();
    gencertdialog->show();
    
}

/** Loads own personal status */
void PeersDialog::loadmypersonalstatus()
{
  
  ui.mypersonalstatuslabel->setText(QString::fromStdString(rsMsgs->getCustomStateString()));
}

void PeersDialog::statusmessage()
{
    static StatusMessage *statusmsgdialog = new StatusMessage();
    statusmsgdialog->show();
}
