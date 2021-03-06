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


#ifndef _SHAREMANAGER_H
#define _SHAREMANAGER_H

#include <QDialog>
#include <QFileDialog>

#include "ui_ShareManager.h"

class ShareManager : public QDialog
{
  Q_OBJECT

public:
  /** Default constructor */
  ShareManager( QWidget *parent = 0, Qt::WFlags flags = 0);
  /** Default destructor */

  /** Loads the settings for this page */
  void load();
  bool messageBoxOk(QString);

public slots:

protected:

private slots:

  /** Create the context popup menu and it's submenus */
  void shareddirListCostumPopupMenu( QPoint point );

  void addShareDirectory();
  void removeShareDirectory();
  
private:



  /** Define the popup menus for the Context menu */
  QMenu* contextMnu;
    /** Defines the actions for the context menu */
  QAction* removeAct;

  /** Qt Designer generated object */
  Ui::ShareManager ui;
};

#endif

