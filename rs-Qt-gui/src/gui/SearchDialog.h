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

#ifndef _SEARCHDIALOG_H
#define _SEARCHDIALOG_H

#include <QFileDialog>

//#include <config/rsharesettings.h>

#include "mainpage.h"
#include "ui_SearchDialog.h"

class SearchDialog : public MainPage 
{
  Q_OBJECT

public:
  /** Default Constructor */
  SearchDialog(QWidget *parent = 0);
  /** Default Destructor */



private slots:

  /** Create the context popup menu and it's submenus */
  void searchtableWidgetCostumPopupMenu( QPoint point );
  
  void searchtableWidget2CostumPopupMenu( QPoint point );
  
  void download();

  void broadcastonchannel();
  
  void recommendtofriends();
  
  
  void searchremove();
  
  void searchremoveall();
  
private:

  /** Define the popup menus for the Context menu */
  QMenu* contextMnu;
  
  QMenu* contextMnu2;
  
  /** Defines the actions for the context menu */
  QAction* downloadAct;
  QAction* broadcastonchannelAct;
  QAction* recommendtofriendsAct;
  
  QAction* searchremoveAct;
  QAction* searchremoveallAct;
  
  QTableWidget *searchtableWidget;
  QTableWidget *searchtablewidget2;
  

  /** Qt Designer generated object */
  Ui::SearchDialog ui;
};

#endif

