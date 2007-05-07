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
      
      configurefriendAct = new QAction( tr( "Configure Friend" ), this );
      connect( configurefriendAct , SIGNAL( triggered() ), this, SLOT( configurefriend2() ) );
      
      exportfriendAct = new QAction(QIcon(IMAGE_EXPIORTFRIEND), tr( "Export Friend" ), this );
      connect( exportfriendAct , SIGNAL( triggered() ), this, SLOT( exportfriend2() ) );
      
      removefriendAct = new QAction(QIcon(IMAGE_REMOVEFRIEND), tr( "Remove Friend" ), this );
      connect( removefriendAct , SIGNAL( triggered() ), this, SLOT( removefriend2() ) );


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
      
}




/** Open a QFileDialog to browse for export a file. */
void MessengerWindow::exportfriend2()
{

}

void MessengerWindow::chatfriend2()
{
    //static PopupChatDialog *popupchatdialog = new PopupChatDialog();
    //popupchatdialog->show();
       

}


void MessengerWindow::removefriend2()
{
   
}


void MessengerWindow::allowfriend2()
{
	
}


void MessengerWindow::connectfriend2()
{
	
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

