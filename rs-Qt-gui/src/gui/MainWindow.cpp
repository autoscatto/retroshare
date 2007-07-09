/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006, 2007 crypton
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

#include <QtGui>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QString>
#include <QtDebug>
#include <QIcon>
#include <QPixmap>

#include <rshare.h>
#include "MainWindow.h"
#include "MessengerWindow.h"
#include "HelpDialog.h"

#include "Preferences/PreferencesWindow.h"
#include "Settings/gsettingswin.h"

#include "rsiface/rsiface.h"

#include "gui/connect/InviteDialog.h"
#include "gui/connect/AddFriendDialog.h"


#define FONT        QFont(tr("Arial"), 8)

/* Images for toolbar icons */
#define IMAGE_NETWORK           ":/images/network32.png"
#define IMAGE_PEERS         	":/images/peers_24x24.png"
#define IMAGE_SEARCH    	    ":/images/filefind.png"
#define IMAGE_TRANSFERS      	":/images/ktorrent.png"
#define IMAGE_FILES   	        ":/images/folder_green.png"
#define IMAGE_CHANNELS       	":/images/konsole.png"
#define IMAGE_PREFERENCES       ":/images/settings16.png"
#define IMAGE_CHAT          	":/images/chats_24x24.png"
#define IMAGE_RETROSHARE        ":/images/RetroShare16.png"
#define IMAGE_ABOUT             ":/images/informations_24x24.png"
#define IMAGE_STATISTIC         ":/images/utilities-system-monitor.png"
#define IMAGE_MESSAGES          ":/images/evolution.png"
#define IMAGE_BWGRAPH           ":/images/ksysguard.png"
#define IMAGE_RSM32             ":/images/rsmessenger32.png"
#define IMAGE_RSM16             ":/images/rsmessenger16.png"
#define IMAGE_CLOSE             ":/images/close_normal.png"

/* uncomment this for release version */
#define RS_RELEASE_VERSION    1

/** Constructor */
MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  /* Invoke the Qt Designer generated QObject setup routine */
  ui.setupUi(this);
  
  	/* Hide Console frame */
	showConsoleFrame(false);
	connect(ui.btnToggleConsole, SIGNAL(toggled(bool)), this, SLOT(showConsoleFrame(bool)));
	
    /* Hide ToolBox frame */
	showtoolboxFrame(true);
	
	// Setting icons
	this->setWindowIcon(QIcon(QString::fromUtf8(":/images/RetroShare16.png")));
	
    /* Create all the dialogs of which we only want one instance */
	_bandwidthGraph = new BandwidthGraph();
	messengerWindow = new MessengerWindow();
        messengerWindow->hide();
	
	connect(ui.addfriendButton, SIGNAL(clicked( bool ) ), this , SLOT( addFriend() ) );
	connect(ui.invitefriendButton, SIGNAL(clicked( bool ) ), this , SLOT( inviteFriend() ) );

	connect(ui.addshareButton, SIGNAL(clicked( bool ) ), this , SLOT( addSharedDirectory() ) );
	connect(ui.optionsButton, SIGNAL(clicked( bool )), this, SLOT( showPreferencesWindow()) );
	
	ui.addfriendButton->setToolTip("Add a Friend");
	ui.invitefriendButton->setToolTip("Invite a Friend");
	ui.addshareButton->setToolTip("Add a Share");
    ui.optionsButton->setToolTip("Options");


  	QPushButton *closeButton = new QPushButton(QIcon(":/images/close_normal.png"), "", ui.tabWidget);
	closeButton->setFixedSize(closeButton->iconSize());
	closeButton->setMaximumSize(QSize(16, 16));
	closeButton->setMinimumSize(QSize(16, 16));
	closeButton->resize(QSize(16, 16));
	closeButton->setToolTip(tr("Exit"));
	ui.tabWidget->setCornerWidget(closeButton);
	ui.tabWidget->cornerWidget()->show();
	connect(closeButton, SIGNAL(clicked()), this, SLOT(closeActiveTab()));
	
    loadStyleSheet("Default");



  /* Create the config pages and actions */
  QActionGroup *grp = new QActionGroup(this);

  ui.stackPages->add(networkDialog = new NetworkDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_NETWORK), tr("Network"), grp));
  
  ui.stackPages->add(peersDialog = new PeersDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_PEERS), tr("Friends"), grp));
                                        
#ifdef RS_RELEASE_VERSION    
  searchDialog = new SearchDialog(ui.stackPages);
  searchDialog -> hide();
#else
  ui.stackPages->add(searchDialog = new SearchDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_SEARCH), tr("Search"), grp));
#endif
                     
  ui.stackPages->add(transfersDialog = new TransfersDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_TRANSFERS), tr("Transfers"), grp));
                     
  ui.stackPages->add(sharedfilesDialog = new SharedFilesDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_FILES), tr("Files"), grp));
                     
  ui.stackPages->add(chatDialog = new ChatDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_CHAT), tr("Chat"), grp));

  ui.stackPages->add(messagesDialog = new MessagesDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_MESSAGES), tr("Messages"), grp));
                     
#ifdef RS_RELEASE_VERSION    
  channelsDialog = new ChannelsDialog(ui.stackPages);
  channelsDialog->hide();
#else
  ui.stackPages->add(channelsDialog = new ChannelsDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_CHANNELS), tr("Channels"), grp));
#endif

  ui.stackPages->add(new HelpDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_ABOUT), tr("About/Help"), grp));
                     
  //ui.stackPages->add(groupsDialog = new GroupsDialog(ui.stackPages),
  //                   createPageAction(QIcon(), tr("Groups"), grp));
                                                              
  //ui.stackPages->add(new StatisticDialog(ui.stackPages),
  //                   createPageAction(QIcon(IMAGE_STATISTIC), tr("Statistics"), grp));

  /* also an empty list of chat windows */
  peersDialog->setChatDialog(chatDialog);
  messengerWindow->setChatDialog(chatDialog);

  /* Create the toolbar */
  ui.toolBar->addActions(grp->actions());
  ui.toolBar->addSeparator();
  connect(grp, SIGNAL(triggered(QAction *)), ui.stackPages, SLOT(showPage(QAction *)));
 
  /* Create and bind the messenger button */
  addAction(new QAction(QIcon(IMAGE_RSM32), tr("Messenger"), ui.toolBar), SLOT(showMessengerWindow()));
 
 #ifdef NO_MORE_OPTIONS_OR_SS

    /* Create and bind the Preferences button */  
  addAction(new QAction(QIcon(IMAGE_PREFERENCES), tr("Options"), ui.toolBar),
            SLOT(showSettings())); 
            

            

#endif

 

  /* Select the first action */
  grp->actions()[0]->setChecked(true);
  
  /* Create the actions that will go in the tray menu */
  createActions();
  
    statusBar()->addWidget(new QLabel(tr("Users: 0  Files: 0 ")));
    statusBar()->addPermanentWidget(new QLabel(tr("Down: 0.0  Up: 0.0 ")));
    statusBar()->addPermanentWidget(new QLabel(tr("Connections: 0/45 ")));
  
/******  
 * This is an annoying warning I get all the time...
 * (no help!)
 *
 *
   if (!QSystemTrayIcon::isSystemTrayAvailable())
            QMessageBox::warning(0, tr("System tray is unavailable"),
                                   tr("System tray unavailable"));
******/

    // Tray icon Menu
    menu = new QMenu(this);
    QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
    toggleVisibilityAction = 
    menu->addAction(QIcon(IMAGE_RETROSHARE), tr("Show/Hide"), this, SLOT(toggleVisibilitycontextmenu()));
    menu->addSeparator();
    menu->addAction(_messengerwindowAct);
    /* bandwidth only in development version */
#ifdef RS_RELEASE_VERSION    
#else
    menu->addAction(_bandwidthAct);
#endif
    menu->addAction(_prefsAct);
    menu->addSeparator();
    menu->addAction("Minimize", this, SLOT(showMinimized()));
    menu->addAction("Maximize", this, SLOT(showMaximized()));
    menu->addSeparator();
    menu->addAction(QIcon(IMAGE_CLOSE), tr("&Quit"), qApp, SLOT(quit()));
    // End of Icon Menu
    
    // Create the tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("RetroShare");
    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon(IMAGE_RETROSHARE));
    
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(toggleVisibility(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();


}

/** Creates a new action associated with a config page. */
QAction*
MainWindow::createPageAction(QIcon img, QString text, QActionGroup *group)
{
  QAction *action = new QAction(img, text, group);
  action->setCheckable(true);
  action->setFont(FONT);
  return action;
}

/** Adds the given action to the toolbar and hooks its triggered() signal to
 * the specified slot (if given). */
void
MainWindow::addAction(QAction *action, const char *slot)
{
  action->setFont(FONT);
  ui.toolBar->addAction(action);
  connect(action, SIGNAL(triggered()), this, slot);
}

/** Overloads the default show so we can load settings */
void
MainWindow::show()
{
  
  if (!this->isVisible()) {
    QMainWindow::show();
  } else {
    QMainWindow::activateWindow();
    setWindowState(windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
    QMainWindow::raise();
  }
}


/** Shows the config dialog with focus set to the given page. */
void
MainWindow::show(Page page)
{
  /* Show the dialog. */
  show();

  /* Set the focus to the specified page. */
  ui.stackPages->setCurrentIndex((int)page);
}



/***** TOOL BAR FUNCTIONS *****/

/** Add a Friend ShortCut */
void MainWindow::addFriend()
{
	/* call load Certificate */
#if 0
	std::string id;
	if (connectionsDialog)
	{
		id = connectionsDialog->loadneighbour();
	}

	/* call make Friend */
	if (id != "")
	{
		connectionsDialog->showpeerdetails(id);
	}
virtual int NeighLoadPEMString(std::string pem, std::string &id)  = 0;
#else

static  AddFriendDialog *addDialog = 
	new AddFriendDialog(networkDialog, this);

	std::string invite = "";
	addDialog->setInfo(invite);
	addDialog->show();
#endif
}


/** Add a Friend ShortCut */
void MainWindow::inviteFriend()
{
static  InviteDialog *inviteDialog = new InviteDialog(this);

	std::string invite = rsicontrol->NeighGetInvite();
	inviteDialog->setInfo(invite);
	inviteDialog->show();


}

/** Shows Preferences */
void MainWindow::addSharedDirectory()
{
	/* Same Code as in Preferences Window (add Share) */

	QString qdir = QFileDialog::getExistingDirectory(this, tr("Add Shared Directory"), "",
	QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
						   
	/* add it to the server */
	std::string dir = qdir.toStdString();
	if (dir != "")
	{
		rsicontrol -> ConfigAddSharedDir(dir);
		rsicontrol -> ConfigSave();
	}

}

/** Shows Preferences */
void MainWindow::showPreferencesWindow()
{
    static PreferencesWindow* preferencesWindow = new PreferencesWindow(this);
    preferencesWindow->show();
}

/** Shows Options */
void MainWindow::showSettings()
{
    static GSettingsWin *win = new GSettingsWin(this);
    if (win->isHidden())
        win->setNewPage(0);
    win->show();
    win->activateWindow();
}

/** Shows Messenger window */
void MainWindow::showMessengerWindow()
{
    messengerWindow->show();
}

/** Destructor. */
MainWindow::~MainWindow()
{
   delete _prefsAct;
   delete _bandwidthGraph;
   delete _messengerwindowAct;
}

/** Create and bind actions to events. Setup for initial
 * tray menu configuration. */
void 
MainWindow::createActions()
{

  _prefsAct = new QAction(QIcon(IMAGE_PREFERENCES), tr("Options"), this);
  connect(_prefsAct, SIGNAL(triggered()), this, SLOT(showPreferencesWindow()));
    
  _bandwidthAct = new QAction(QIcon(IMAGE_BWGRAPH), tr("Bandwidth Graph"), this);
  connect(_bandwidthAct, SIGNAL(triggered()), 
          _bandwidthGraph, SLOT(showWindow()));
          
  _messengerwindowAct = new QAction(QIcon(IMAGE_RSM16), tr("Open Messenger"), this);
  connect(_messengerwindowAct, SIGNAL(triggered()),this, SLOT(showMessengerWindow()));
         
          
  connect(ui.btntoggletoolbox, SIGNAL(toggled(bool)), this, SLOT(showtoolboxFrame(bool)));
  
}



void MainWindow::closeEvent(QCloseEvent *e)
{
    static bool firstTime = true;

    if (trayIcon->isVisible()) {
        if (firstTime)
	{
/*****
          QMessageBox::information(this, tr("RetroShare System tray"),
                    tr("Application will continue running. Quit using context menu in the system tray"));
*****/
	  firstTime = false;
	}
        hide();
        e->ignore();
    }

}

void MainWindow::updateMenu()
{
    toggleVisibilityAction->setText(isVisible() ? tr("Hide") : tr("Show"));
}

void MainWindow::toggleVisibility(QSystemTrayIcon::ActivationReason e)
{
  if(e == QSystemTrayIcon::Trigger || e == QSystemTrayIcon::DoubleClick){
    if(isHidden()){
      show();
      if(isMinimized()){
        if(isMaximized()){
          showMaximized();
        }else{
          showNormal();
        }
      }
      raise();
      activateWindow();
    }else{
      hide();
    }
  }
}

void MainWindow::toggleVisibilitycontextmenu()
{
    if (isVisible())
         hide();
     else
         show();
}


/**
 Toggles the Console pane on and off, changes toggle button text
*/
void MainWindow::showConsoleFrame(bool show)
{
	if (show) {
		ui.frmConsole->setVisible(true);
		ui.btnToggleConsole->setChecked(true);
		ui.btnToggleConsole->setToolTip("Hide Console");
	} else {
		ui.frmConsole->setVisible(false);
		ui.btnToggleConsole->setChecked(false);
		ui.btnToggleConsole->setToolTip("Show Console");
	}
}

/**
 Toggles the ToolBox on and off, changes toggle button text
*/
void MainWindow::showtoolboxFrame(bool show)
{
	if (show) {
		ui.toolboxframe->setVisible(true);
		ui.btntoggletoolbox->setChecked(true);
		ui.btntoggletoolbox->setToolTip("Hide ToolBox");
		ui.btntoggletoolbox->setIcon(QIcon(":images/hide_toolbox_frame.png"));
	} else {
		ui.toolboxframe->setVisible(false);
		ui.btntoggletoolbox->setChecked(false);
		ui.btntoggletoolbox->setToolTip("Show ToolBox");
		ui.btntoggletoolbox->setIcon(QIcon(":images/show_toolbox_frame.png"));
	}
}


void MainWindow::on_styleSheetCombo_activated(const QString &sheetName)
{
    loadStyleSheet(sheetName);
}

void MainWindow::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/qss/" + sheetName.toLower() + ".qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());

    
    qApp->setStyleSheet(styleSheet);
    
}


