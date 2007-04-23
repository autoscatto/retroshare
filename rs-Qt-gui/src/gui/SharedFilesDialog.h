/****************************************************************
 *  RShare is distributed under the following license:
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

#ifndef _SHAREDFILESDIALOG_H
#define _SHAREDFILESDIALOG_H

#include <QFileDialog>

//#include <config/rsharesettings.h>

#include "mainpage.h"
#include "ui_SharedFilesDialog.h"

#include "rsiface/rstypes.h"
#include "rsiface/RemoteDirModel.h"

class SharedFilesDialog : public MainPage 
{
  Q_OBJECT

public:
  /** Default Constructor */
  SharedFilesDialog(QWidget *parent = 0);
  /** Default Destructor */

#ifdef NO_MORE
void insertFiles(bool local);
void addDirectories(const DirInfo &root, QTreeWidgetItem *parent);

void syncDirectories(bool update_local);
void syncDirectory(const DirInfo &root, QTreeWidgetItem *parent);

#endif

	/* For handling the model updates */
void  preModDirectories(bool update_local);
void     ModDirectories(bool update_local);




private slots:

  /** Create the context popup menu and it's submenus */
  void shareddirtreeviewCostumPopupMenu( QPoint point );
  
  void shareddirtreeWidgetCostumPopupMenu( QPoint point );
  
  void downloadRemoteSelected();
  void addMsgRemoteSelected();

  void recommendfile();
  void openfile();
  void openfolder();

#ifdef NO_MORE
  void checkForLocalDirRequest( QTreeWidgetItem * );
  void checkForRemoteDirRequest( QTreeWidgetItem * );

#endif
  
  

private:
  /** Define the popup menus for the Context menu */
  QMenu* contextMnu;
  
  QMenu* contextMnu2;
  
  /** Defines the actions for the context menu for QTreeView */
  QAction* downloadAct;
  QAction* addMsgAct;
  
  /** Defines the actions for the context menu for QTreeWidget */
  QAction* openfileAct;
  QAction* openfolderAct;
  
  
  QTreeView *shareddirtreeview;
  
#ifdef NO_MORE
  QTreeWidget *shareddirtreeWidget;
#endif

  /** Qt Designer generated object */
  Ui::SharedFilesDialog ui;

  /* RemoteDirModel */
  RemoteDirModel *model;
  RemoteDirModel *localModel;
};

#endif

