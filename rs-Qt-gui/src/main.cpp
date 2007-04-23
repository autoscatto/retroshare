/****************************************************************
 *  RetroShare QT Gui is distributed under the following license:
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
#include <QObject>
#include <rshare.h>
#include <gui/MainWindow.h>
#include <gui/StartDialog.h>

#include <util/process.h>
#include <util/string.h>
#include "rsiface/rsiface.h"

#include "rsiface/notifyqt.h"

RsIface   *rsiface    = NULL;
RsControl *rsicontrol = NULL;

int main(int argc, char *argv[])
{ 

  QStringList args = char_array_to_stringlist(argv+1, argc-1);
  
  //Q_INIT_RESOURCE(stylesheet);

	rsiface = NULL;

        /* RetroShare Core Objects */
        RsInit *config = InitRsConfig();
        bool okStart = InitRetroShare(argc, argv, config);


	/* Setup The GUI Stuff */
	Rshare rshare(args, argc, argv);

	/* Login Dialog */
  	if (!okStart)
	{
		StartDialog *sd = new StartDialog(config);
		sd->show();

		while(sd -> isVisible())
		{
			rshare.processEvents();
		}
	}
	else
	{
        	LoadCertificates(config);
	}

        NotifyQt   *notify = new NotifyQt();
        RsIface *iface = createRsIface(*notify);
        RsControl *rsServer = createRsControl(*iface, *notify);

        notify->setRsIface(iface);

  MainWindow w;
  w.show();

	/* Attach the Dialogs, to the Notify Class */
        notify->setConnectionDialog(w.connectionsDialog);
        notify->setPeersDialog(w.peersDialog);
        notify->setDirDialog(w.sharedfilesDialog);
        notify->setTransfersDialog(w.transfersDialog);
        notify->setChatDialog(w.chatDialog);
        notify->setMessagesDialog(w.messagesDialog);
        notify->setChannelsDialog(w.channelsDialog);

        rsServer -> StartupRetroShare(config);
        CleanupRsConfig(config);

	/* save to the global variables */
	rsiface = iface;
	rsicontrol = rsServer;

	/* Startup a Timer to keep the gui's updated */
	QTimer *timer = new QTimer(&w);
	timer -> connect(timer, SIGNAL(timeout()), notify, SLOT(UpdateGUI()));
        timer->start(1000);
	
	/* dive into the endless loop */
  return rshare.exec();
}





