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

#ifndef _MESSENGERWINDOW_H
#define _MESSENGERWINDOW_H
#include <QFileDialog>

#include "mainpage.h"
#include "ui_MessengerWindow.h"



class MessengerWindow : public QWidget
{
  Q_OBJECT

public:
  /** Default Constructor */
  MessengerWindow(QWidget *parent = 0);
  /** Default Destructor */

  void  insertPeers2();

protected:
  void closeEvent (QCloseEvent * event);


private slots:

  /** Create the context popup menu and it's submenus */
  void messengertreeWidgetCostumPopupMenu( QPoint point );
  
  /** Export friend in Friends Dialog */
  void exportfriend2();
  /** Remove friend  */
  void removefriend2();
  /** start a chat with a friend **/
  void chatfriend2();

  void configurefriend2();

  /** RsServer Friend Calls */
  void allowfriend2();
  void connectfriend2();
  void setaddressfriend2();
  void trustfriend2();

  
private:

  /* Worker Functions */
  /* (1) Update Display */

  /* (2) Utility Fns */
  //QTreeWidgetItem *getCurrentPeer2();

  /** Define the popup menus for the Context menu */
  QMenu* contextMnu;
   /** Defines the actions for the context menu */
  QAction* chatAct;
  QAction* connectfriendAct;
  QAction* configurefriendAct;
  QAction* exportfriendAct;
  QAction* removefriendAct;

  QTreeView *messengertreeWidget;
  

  /** Qt Designer generated object */
  Ui::MessengerWindow ui;
};

#endif

