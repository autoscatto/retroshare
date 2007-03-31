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

#ifndef _MainWindow_H
#define _MainWindow_H

#include <QtGui>
#include <QMainWindow>
#include <QFileDialog>
#include <QSystemTrayIcon>


#include "ConnectionsDialog.h"
#include "PeersDialog.h"
#include "SearchDialog.h"
#include "TransfersDialog.h"
#include "MessagesDialog.h"
#include "ChannelsDialog.h"
#include "ChatDialog.h"
#include "SharedFilesDialog.h"
#include "StatisticDialog.h"

#include "Preferences/PreferencesWindow.h"
#include "Settings/gsettingswin.h"


#include "ui_MainWindow.h"


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
   /** Main dialog pages. */
  enum Page {
    Network            = 0,  /** Connections page. */
    Friends            = 1,  /** Peers page. */
    SharedDirectories  = 2,  /** Shared Directories page. */
    Search 		       = 3,  /** Search page. */
    Transfers          = 4,  /** Transfers page. */
    Chat               = 5,  /** Chat page. */
    Messages           = 6,  /** Messages page. */
    Channels           = 7,  /** Channels page. */
    Statistics         = 8  /** Statistic page. */
    
  };

  /** Default Constructor */
  MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
  
    /** Destructor. */
  ~MainWindow();

  /* A Bit of a Hack... but public variables for 
   * the dialogs, so we can add them to the 
   * Notify Class...
   */

  ConnectionsDialog *connectionsDialog;
  PeersDialog       *peersDialog;
  SearchDialog      *searchDialog;
  TransfersDialog   *transfersDialog;
  ChatDialog        *chatDialog;
  MessagesDialog    *messagesDialog;
  ChannelsDialog    *channelsDialog;
  SharedFilesDialog *sharedfilesDialog;
  //GroupsDialog      *groupsDialog;
  //StatisticDialog   *statisticDialog;


public slots:
  /** Called when this dialog is to be displayed */
  void show();
  /** Shows the config dialog with focus set to the given page. */
  void show(Page page);
  
  

private slots:

    void updateMenu();
    void toggleVisibility();
 
    void showPreferencesWindow();
    void showSettings();
    
    /** Called when console button is toggled */
	void showConsoleFrame(bool show);
	
	/** Called when console button is toggled */
	void showtoolboxFrame(bool show);

    
	//void addShareDirectory();


protected:
    void closeEvent(QCloseEvent *);



private:
    /** Loads the current configuration settings */
  //void loadSettings();


  /** Create the actions on the tray menu or menubar */
  void createActions();
 

  /** Defines the actions for the tray menu */
  QAction* prefsAct;
  

  /** Creates a new action for a config page. */
  QAction* createPageAction(QIcon img, QString text, QActionGroup *group);
  /** Adds a new action to the toolbar. */
  void addAction(QAction *action, const char *slot = 0);
  

    QSystemTrayIcon *trayIcon;
    QAction *toggleVisibilityAction;
    QMenu *menu;
    
    PreferencesWindow* _preferencesWindow;
  



  /** Qt Designer generated object */
  Ui::MainWindow ui;
};

#endif

