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


#include <rshare.h>
#include "ServerDialog.h"
#include <iostream>

#include "rsiface/rsiface.h"



/** Constructor */
ServerDialog::ServerDialog(QWidget *parent)
: ConfigPage(parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

 /* Create RshareSettings object */
  _settings = new RshareSettings();
  connect(ui.serverButton, SIGNAL(clicked( bool )), this, SLOT( saveAddresses()) );

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

/** Saves the changes on this page */
bool
ServerDialog::save(QString &errmsg)
{

/* save the server address */
/* save local address */
/* save the url for DNS access */

/* restart server */

/* save all? */
 return true;
}


/** Loads the settings for this page */
void
ServerDialog::load()
{
	/* get the shared directories */
	rsiface->lockData(); /* Lock Interface */
	
/* set local address */
	ui.localAddress->setText(QString::fromStdString(rsiface->getConfig().localAddr));
	ui.localPort -> setValue(rsiface->getConfig().localPort);
/* set the server address */
	ui.extAddress->setText(QString::fromStdString(rsiface->getConfig().extAddr));
	ui.extPort -> setValue(rsiface->getConfig().extPort);
/* set the url for DNS access */
	ui.extName->setText(QString::fromStdString(rsiface->getConfig().extName));
	ui.chkFirewall  ->setChecked(rsiface->getConfig().firewalled);
	ui.chkForwarded ->setChecked(rsiface->getConfig().forwardPort);

	ui.totalRate->setValue(rsiface->getConfig().maxDataRate);
	ui.indivRate->setValue(rsiface->getConfig().maxIndivDataRate);

	rsiface->unlockData(); /* UnLock Interface */
}

void ServerDialog::saveAddresses()
{
	QString str;

	rsicontrol->ConfigSetLocalAddr(ui.localAddress->text().toStdString(), ui.localPort->value());
	rsicontrol->ConfigSetLanConfig(ui.chkFirewall->isChecked(), ui.chkForwarded->isChecked());
	rsicontrol->ConfigSetExtAddr(ui.extAddress->text().toStdString(), ui.extPort->value());
	rsicontrol->ConfigSetExtName(ui.extName->text().toStdString());
	rsicontrol->ConfigSetDataRates( ui.totalRate->value(),  ui.indivRate->value() );

	if (save(str))
	{
		std::cerr << "ServerDialog::saveAddresses() Success!" << std::endl;
	}
	else
	{
		std::cerr << "ServerDialog::saveAddresses() Failed" << std::endl;
	}
	load();
}

