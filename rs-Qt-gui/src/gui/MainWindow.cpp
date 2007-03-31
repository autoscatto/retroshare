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

#include <QtGui>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QString>
#include <QtDebug>
#include <QIcon>
#include <QPixmap>

#include <rshare.h>
#include "MainWindow.h"
#include "Preferences/PreferencesWindow.h"
#include "Settings/gsettingswin.h"
#include "config/gconfig.h"


#define FONT        QFont(tr("Arial"), 8)

/* Images for toolbar icons */
#define IMAGE_NETWORK           ":/images/network32.png"
#define IMAGE_PEERS        		":/images/peers_24x24.png"
#define IMAGE_SEARCH    		":/images/filefind.png"
#define IMAGE_TRANSFERS      	":/images/ktorrent.png"
#define IMAGE_FILES   	        ":/images/folder_green.png"
#define IMAGE_CHANNELS       	":/images/konsole.png"
#define IMAGE_PREFERENCES       ":/images/settings.png"
#define IMAGE_CHAT          	":/images/chats_24x24.png"
#define IMAGE_RETROSHARE        ":/images/RetroShare16.png"
#define IMAGE_INFORMATIONS      ":/images/informations_24x24.png"
#define IMAGE_STATISTIC         ":/images/utilities-system-monitor.png"
#define IMAGE_MESSAGES          ":/images/evolution.png"


/** Constructor */
MainWindow::MainWindow(QWidget* parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  /* Invoke the Qt Designer generated QObject setup routine */
  ui.setupUi(this);
  
  GConfig config;
  config.loadWidgetInformation(this);
  
  	/* Hide Console frame */
	showConsoleFrame(false);
	connect(ui.btnToggleConsole, SIGNAL(toggled(bool)), this, SLOT(showConsoleFrame(bool)));
	
    /* Hide ToolBox frame */
	showtoolboxFrame(true);
	
	connect(ui.optionsButton, SIGNAL(clicked( bool )), this, SLOT( showPreferencesWindow()) );
	//connect(ui.addshareButton, SIGNAL(clicked( bool ) ), this , SLOT( addaShareDirectory() ) );

  
  	QPushButton *closeButton = new QPushButton(QIcon(":/images/close_normal.png"), "", ui.tabWidget);
	closeButton->setFixedSize(closeButton->iconSize());
	closeButton->setMaximumSize(QSize(16, 16));
	closeButton->setMinimumSize(QSize(16, 16));
	closeButton->resize(QSize(16, 16));
	closeButton->setToolTip(tr("Exit"));
	ui.tabWidget->setCornerWidget(closeButton);
	ui.tabWidget->cornerWidget()->show();
	connect(closeButton, SIGNAL(clicked()), this, SLOT(closeActiveTab()));



  /* Create the config pages and actions */
  QActionGroup *grp = new QActionGroup(this);

  ui.stackPages->add(connectionsDialog = new ConnectionsDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_NETWORK), tr("Network"), grp));
  
  ui.stackPages->add(peersDialog = new PeersDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_PEERS), tr("Friends"), grp));
                                        
  ui.stackPages->add(searchDialog = new SearchDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_SEARCH), tr("Search"), grp));
                     
  ui.stackPages->add(transfersDialog = new TransfersDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_TRANSFERS), tr("Transfers"), grp));
                     
  ui.stackPages->add(sharedfilesDialog = new SharedFilesDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_FILES), tr("Files"), grp));
                     
  ui.stackPages->add(chatDialog = new ChatDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_CHAT), tr("Chat"), grp));

  ui.stackPages->add(messagesDialog = new MessagesDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_MESSAGES), tr("Messages"), grp));
                     
  ui.stackPages->add(channelsDialog = new ChannelsDialog(ui.stackPages),
                     createPageAction(QIcon(IMAGE_CHANNELS), tr("Channels"), grp));
                     
  //ui.stackPages->add(groupsDialog = new GroupsDialog(ui.stackPages),
  //                   createPageAction(QIcon(), tr("Groups"), grp));

                                         
                     
  //ui.stackPages->add(new StatisticDialog(ui.stackPages),
  //                   createPageAction(QIcon(IMAGE_STATISTIC), tr("Statistics"), grp));
  
  /* Create the toolbar */
  ui.toolBar->addActions(grp->actions());
  ui.toolBar->addSeparator();
  connect(grp, SIGNAL(triggered(QAction *)), ui.stackPages, SLOT(showPage(QAction *)));
 
     	/* The Real grouptab-button */
	//addAction(new QAction(QIcon(), tr("Grptab"), ui.toolBar), SLOT(addGroup()));
 
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
  
  
   if (!QSystemTrayIcon::isSystemTrayAvailable())
            QMessageBox::warning(0, tr("System tray is unavailable"),
                                   tr("System tray unavailable"));

    // Create the menu that will be used for the context menu
    menu = new QMenu(this);
    QObject::connect(menu, SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
    toggleVisibilityAction = 
    menu->addAction("Show/Hide", this, SLOT(toggleVisibility()));
    menu->addAction("Preferences", this, SLOT(showPreferencesWindow()));
    menu->addAction("Minimize", this, SLOT(showMinimized()));
    menu->addAction("Maximize", this, SLOT(showMaximized()));
    menu->addSeparator();
    menu->addAction("&Quit", qApp, SLOT(quit()));

    // Create the tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setToolTip("RetroShare");
    trayIcon->setContextMenu(menu);
    trayIcon->setIcon(QIcon(IMAGE_RETROSHARE));
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


/** Destructor. */
MainWindow::~MainWindow()
{


}

/** Create and bind actions to events. Setup for initial
 * tray menu configuration. */
void 
MainWindow::createActions()
{

  prefsAct = new QAction(QIcon(IMAGE_PREFERENCES), tr("Preferences"), this);
  connect(prefsAct, SIGNAL(triggered()), this, SLOT(showPreferencesWindow()));
  
  connect(ui.btntoggletoolbox, SIGNAL(toggled(bool)), this, SLOT(showtoolboxFrame(bool)));
  
}



void MainWindow::closeEvent(QCloseEvent *e)
{
    if (trayIcon->isVisible()) {
        QMessageBox::information(this, tr("RetroShare System tray"),
                    tr("Application will continue running. Quit using context menu in the system tray"));
        hide();
        e->ignore();
    }

}

void MainWindow::updateMenu()
{
    toggleVisibilityAction->setText(isVisible() ? tr("Hide") : tr("Show"));
}

void MainWindow::toggleVisibility()
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
		ui.btnToggleConsole->setToolTip("Hide ChatterBox");
	} else {
		ui.frmConsole->setVisible(false);
		ui.btnToggleConsole->setChecked(false);
		ui.btnToggleConsole->setToolTip("Show ChatterBox");
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





