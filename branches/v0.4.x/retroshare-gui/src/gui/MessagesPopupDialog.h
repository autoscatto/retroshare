/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2009,The RetroShare Team
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

#ifndef _MESSAGESPOPUPDIALOG_H
#define _MESSAGESPOPUPDIALOG_H

#include <QFileDialog>

#include "ui_MessagesPopupDialog.h"

class MessagesPopupDialog : public QMainWindow 
{
  Q_OBJECT

public:
  /** Default Constructor */
  MessagesPopupDialog(QWidget *parent = 0, Qt::WFlags flags = 0);
  /** Default Destructor */


 void insertMessages();
 void insertMsgTxtAndFiles();


 virtual void keyPressEvent(QKeyEvent *) ;
private slots:

  /** Create the context popup menu and it's submenus */
  void messageslistWidgetCostumPopupMenu( QPoint point ); 
  void msgfilelistWidgetCostumPopupMenu(QPoint);  

  void changeBox( int newrow );
  void updateMessages ( QTreeWidgetItem * item, int column );

  void newmessage();

  void replytomessage();
  void forwardmessage();

  void print();
  void printpreview();
  
  void removemessage();
  void markMsgAsRead();  
  
  void getcurrentrecommended();
  void getallrecommended();

  /* handle splitter */
  void togglefileview();

private:

  bool getCurrentMsg(std::string &cid, std::string &mid);

  std::string mCurrCertId;
  std::string mCurrMsgId;

  /** Define the popup menus for the Context menu */
  QMenu* contextMnu;
  
   /** Defines the actions for the context menu */
  QAction* newmsgAct;
  QAction* replytomsgAct;
  QAction* forwardmsgAct;
  QAction* removemsgAct;

  QAction* getRecAct;
  QAction* getAllRecAct;
  
  /** Qt Designer generated object */
  Ui::MessagesPopupDialog ui;
};

#endif

