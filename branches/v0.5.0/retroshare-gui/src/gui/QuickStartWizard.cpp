/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006-2010,  RetroShare Team
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

#include "QuickStartWizard.h"

#include <QFileDialog>
#include <QSettings>
#include <QCheckBox>
#include <QMessageBox>
#include <QComboBox>
#include <QHeaderView>

#include "rsiface/rsfiles.h"
#include "rsiface/rsiface.h"
#include "rsiface/rspeers.h"


QuickStartWizard::QuickStartWizard(QWidget *parent) :
    QDialog(parent)
{
    ui.setupUi(this);

	  ui.pagesWizard->setCurrentIndex(0);
	  
	  /* Create RshareSettings object */
    _settings = new RshareSettings();
    
          loadNetwork();
	  loadShare();
	  loadGeneral();

	  
//	   ui.checkBoxF2FRouting->setChecked(true) ;
//	   ui.checkBoxF2FRouting->setEnabled(false) ;
	  
	  connect( ui.netModeComboBox, SIGNAL( activated ( int ) ), this, SLOT( toggleUPnP( ) ) );
//	  connect( ui.checkBoxTunnelConnection, SIGNAL( toggled( bool ) ), this, SLOT( toggleTunnelConnection(bool) ) );
	  
//	  bool b = rsPeers->getAllowTunnelConnection() ;
//    ui.checkBoxTunnelConnection->setChecked(b) ;
    
    ui.shareddirList->horizontalHeader()->setResizeMode( 0,QHeaderView::Stretch);
    ui.shareddirList->horizontalHeader()->setResizeMode( 2,QHeaderView::Interactive); 
 
    ui.shareddirList->horizontalHeader()->resizeSection( 0, 360 );
    ui.shareddirList->horizontalHeader()->setStretchLastSection(false);
	  
  /* Hide platform specific features */
#ifndef Q_WS_WIN
  ui.checkBoxRunRetroshareAtSystemStartup->setVisible(false);

#endif	  
}

QuickStartWizard::~QuickStartWizard()
{
    //delete ui;
}

void QuickStartWizard::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui.retranslateUi(this);
        break;
    default:
        break;
    }
}

void QuickStartWizard::on_pushButtonWelcomeNext_clicked()
{
        ui.pagesWizard->setCurrentIndex(1);
}

void QuickStartWizard::on_pushButtonWelcomeExit_clicked()
{
        close();
}

void QuickStartWizard::on_pushButtonConnectionBack_clicked()
{
        ui.pagesWizard->setCurrentIndex(0);
}

void QuickStartWizard::on_pushButtonConnectionNext_clicked()
{
        /* Check if netMode has changed */
        int netMode = 0;
        switch(ui.netModeComboBox->currentIndex())
        {
                case 2:
                        netMode = RS_NETMODE_EXT;
                        break;
                case 1:
                        netMode = RS_NETMODE_UDP;
                        break;
                default:
                case 0:
                        netMode = RS_NETMODE_UPNP;
                        break;
        }
        std::cerr << "ui.netModeComboBox->currentIndex()" << ui.netModeComboBox->currentIndex() << std::endl;
        rsPeers->setNetworkMode(rsPeers->getOwnId(), netMode);

        /* Check if vis has changed */
        int visState = 0;
        if (0 == ui.discoveryComboBox->currentIndex())
        {
                visState |= RS_VS_DISC_ON;
        }

        RsPeerDetails detail;
        if (!rsPeers->getPeerDetails(rsPeers->getOwnId(), detail))
        {
                return;
        }
        if (visState != detail.visState)
        {
                rsPeers->setVisState(rsPeers->getOwnId(), visState);
        }
        rsicontrol->ConfigSetDataRates( ui.doubleSpinBoxDownloadSpeed->value(), ui.doubleSpinBoxUploadSpeed->value() );

        ui.pagesWizard->setCurrentIndex(2);
}

void QuickStartWizard::on_pushButtonConnectionExit_clicked()
{
        on_pushButtonConnectionNext_clicked();
        close();
}

void QuickStartWizard::on_pushButtonSharesBack_clicked()
{
        ui.pagesWizard->setCurrentIndex(1);
}

void QuickStartWizard::on_pushButtonSharesNext_clicked()
{
        ui.pagesWizard->setCurrentIndex(3);
}

void QuickStartWizard::on_pushButtonSharesExit_clicked()
{
        close();
}

void QuickStartWizard::on_pushButtonSystemBack_clicked()
{
        ui.pagesWizard->setCurrentIndex(2);
}

void QuickStartWizard::on_pushButtonSystemFinish_clicked()
{
	
  _settings->setValue(QString::fromUtf8("StartMinimized"), startMinimized());

  _settings->setValue(QString::fromUtf8("doQuit"), quitbox());
  
  _settings->setRunRetroshareOnBoot(ui.checkBoxRunRetroshareAtSystemStartup->isChecked());
  
  saveChanges();
  
  close();
}

void QuickStartWizard::on_pushButtonSystemExit_clicked()
{
	close();
}

void QuickStartWizard::on_pushButtonSharesAdd_clicked()
{
/* select a dir
	 */


	QString qdir = QFileDialog::getExistingDirectory(this, tr("Select A Folder To Share"), "",
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	/* add it to the server */
	std::string dir = qdir.toStdString();
	if (dir != "")
	{
		SharedDirInfo sdi ;
		sdi.filename = dir ;
		sdi.shareflags = RS_FILE_HINTS_NETWORK_WIDE | RS_FILE_HINTS_BROWSABLE ;

		rsFiles->addSharedDirectory(sdi);

		messageBoxOk(tr("Shared Directory Added!"));
		loadShare();
	}
}

void QuickStartWizard::on_pushButtonSharesRemove_clicked()
{
	/* id current dir */
	/* ask for removal */
	QTableWidget *listWidget = ui.shareddirList;
	int row = listWidget -> currentRow();
	QTableWidgetItem *qdir = listWidget->item(row,0) ;

	QString queryWrn;
	queryWrn.clear();
	queryWrn.append(tr("Do you really want to stop sharing this directory ? "));

	if (qdir)
	{
		if ((QMessageBox::question(this, tr("Warning!"),queryWrn,QMessageBox::Ok|QMessageBox::No, QMessageBox::Ok))== QMessageBox::Ok)
		{
			rsFiles->removeSharedDirectory( qdir->text().toStdString());
			loadShare();
		}
		else
		return;
	}
}

void QuickStartWizard::loadShare()
{
	std::cerr << "ShareManager:: In load !!!!!" << std::endl ;

	std::list<SharedDirInfo>::const_iterator it;
	std::list<SharedDirInfo> dirs;
	rsFiles->getSharedDirectories(dirs);

	/* get a link to the table */
	QTableWidget *listWidget = ui.shareddirList;

	/* remove old items ??? */
	listWidget->clearContents() ;
	listWidget->setRowCount(0) ;

	connect(this,SIGNAL(itemClicked(QTableWidgetItem*)),this,SLOT(updateFlags(QTableWidgetItem*))) ;

	int row=0 ;
	for(it = dirs.begin(); it != dirs.end(); it++,++row)
	{
		listWidget->insertRow(row) ;
		listWidget->setItem(row,0,new QTableWidgetItem(QString::fromStdString((*it).filename)));
#ifdef USE_COMBOBOX
		QComboBox *cb = new QComboBox ;
		cb->addItem(QString("Network Wide")) ;
		cb->addItem(QString("Browsable")) ;
		cb->addItem(QString("Universal")) ;

		cb->setToolTip(QString("Decide here whether this directory is\n* Network Wide: \tanonymously shared over the network (including your friends)\n* Browsable: \tbrowsable by your friends\n* Universal: \t\tboth")) ;

		// TODO
		//  - set combobox current value depending on what rsFiles reports.
		//  - use a signal mapper to get the correct row that contains the combo box sending the signal:
		//  		mapper = new SignalMapper(this) ;
		//
		//  		for(all cb)
		//  		{
		//  			signalMapper->setMapping(cb,...)
		//  		}
		//
		int index = 0 ;
		index += ((*it).shareflags & RS_FILE_HINTS_NETWORK_WIDE) > 0 ;
		index += (((*it).shareflags & RS_FILE_HINTS_BROWSABLE) > 0) * 2 ;
		listWidget->setCellWidget(row,1,cb);

		if(index < 1 || index > 3)
			std::cerr << "******* ERROR IN FILE SHARING FLAGS. Flags = " << (*it).shareflags << " ***********" << std::endl ;
		else
			index-- ;

		cb->setCurrentIndex(index) ;
#else
		QCheckBox *cb1 = new QCheckBox ;
		QCheckBox *cb2 = new QCheckBox ;

		cb1->setChecked( (*it).shareflags & RS_FILE_HINTS_NETWORK_WIDE ) ;
		cb2->setChecked( (*it).shareflags & RS_FILE_HINTS_BROWSABLE ) ;

		cb1->setToolTip(QString("If checked, the share is anonymously shared to anybody.")) ;
		cb2->setToolTip(QString("If checked, the share is browsable by your friends.")) ;

		listWidget->setCellWidget(row,1,cb1);
		listWidget->setCellWidget(row,2,cb2);

		QObject::connect(cb1,SIGNAL(toggled(bool)),this,SLOT(updateFlags(bool))) ;
		QObject::connect(cb2,SIGNAL(toggled(bool)),this,SLOT(updateFlags(bool))) ;
#endif
	}

	//ui.incomingDir->setText(QString::fromStdString(rsFiles->getDownloadDirectory()));

	listWidget->update(); /* update display */
	update();
}

void QuickStartWizard::updateFlags(bool b)
{
	std::cerr << "Updating flags (b=" << b << ") !!!" << std::endl ;

	std::list<SharedDirInfo>::iterator it;
	std::list<SharedDirInfo> dirs;
	rsFiles->getSharedDirectories(dirs);

	int row=0 ;
	for(it = dirs.begin(); it != dirs.end(); it++,++row)
	{
		std::cerr << "Looking for row=" << row << ", file=" << (*it).filename << ", flags=" << (*it).shareflags << std::endl ;
		uint32_t current_flags = 0 ;
		current_flags |= (dynamic_cast<QCheckBox*>(ui.shareddirList->cellWidget(row,1)))->isChecked()? RS_FILE_HINTS_NETWORK_WIDE:0 ;
		current_flags |= (dynamic_cast<QCheckBox*>(ui.shareddirList->cellWidget(row,2)))->isChecked()? RS_FILE_HINTS_BROWSABLE:0 ;

		if( (*it).shareflags ^ current_flags )
		{
			(*it).shareflags = current_flags ;
			rsFiles->updateShareFlags(*it) ;	// modifies the flags

			std::cout << "Updating share flags for directory " << (*it).filename << std::endl ;
		}
	}
}

bool QuickStartWizard::messageBoxOk(QString msg)
 {
    QMessageBox mb("Share Manager InfoBox!",msg,QMessageBox::Information,QMessageBox::Ok,0,0);
    mb.setButtonText( QMessageBox::Ok, "OK" );
    mb.exec();
    return true;
 }

/*void QuickStartWizard::showEvent(QShowEvent *event)
{
	if (!event->spontaneous())
	{
		loadsharelist();
	}
}*/

/** Loads the settings for this page */
void
QuickStartWizard::loadGeneral()
{
  ui.checkBoxRunRetroshareAtSystemStartup->setChecked(_settings->runRetroshareOnBoot());

  ui.checkBoxStartMinimized->setChecked(_settings->value(QString::fromUtf8("StartMinimized"), false).toBool());

  ui.checkBoxQuit->setChecked(_settings->value(QString::fromUtf8("doQuit"), false).toBool());
  
        //ui.checkBoxQuickWizard->setChecked(_settings->value(QString::fromUtf8("FirstRun"), false).toBool());
}

bool QuickStartWizard::quitbox() const {
  if(ui.checkBoxQuit->isChecked()) return true;
  return ui.checkBoxQuit->isChecked();
}

bool QuickStartWizard::startMinimized() const {
  if(ui.checkBoxStartMinimized->isChecked()) return true;
  return ui.checkBoxStartMinimized->isChecked();
}

//bool QuickStartWizard::firstRunWizard() const {
//  if(ui.checkBoxQuickWizard->isChecked()) return true;
//  return ui.checkBoxQuickWizard->isChecked();
//}

/** Loads the settings for this page */
void QuickStartWizard::loadNetwork()
{

	/* load up configuration from rsPeers */
	RsPeerDetails detail;
	if (!rsPeers->getPeerDetails(rsPeers->getOwnId(), detail))
	{
		return;
	}

	/* set net mode */
	int netIndex = 0;
	switch(detail.tryNetMode)
	{
		case RS_NETMODE_EXT:
			netIndex = 2;
			break;
		case RS_NETMODE_UDP:
			netIndex = 1;
			break;
		default:
		case RS_NETMODE_UPNP:
			netIndex = 0;
			break;
	}
	ui.netModeComboBox->setCurrentIndex(netIndex);

	netIndex = 1;
	if (detail.visState & RS_VS_DISC_ON)
	{
		netIndex = 0;
	}
	ui.discoveryComboBox->setCurrentIndex(netIndex);

	rsiface->lockData(); /* Lock Interface */

        ui.doubleSpinBoxDownloadSpeed->setValue(rsiface->getConfig().maxDownloadDataRate);
	ui.doubleSpinBoxUploadSpeed->setValue(rsiface->getConfig().maxUploadDataRate);

	rsiface->unlockData(); /* UnLock Interface */
}

void QuickStartWizard::saveChanges()
{
	QString str;

	bool saveAddr = false;


	RsPeerDetails detail;
	std::string ownId = rsPeers->getOwnId();

	if (!rsPeers->getPeerDetails(ownId, detail))
	{
		return;
	}


	/* Check if netMode has changed */
	int netMode = 0;
        int netIndex = ui.netModeComboBox->currentIndex();
        std::cerr << "ui.netModeComboBox->currentIndex()" << ui.netModeComboBox->currentIndex() << std::endl;
        switch(netIndex)
	{
		case 2:
			netMode = RS_NETMODE_EXT;
			break;
		case 1:
			netMode = RS_NETMODE_UDP;
			break;
		default:
		case 0:
                        netMode = RS_NETMODE_UPNP;
			break;
	}
        rsPeers->setNetworkMode(ownId, netMode);

        int visState = 0;
        /* Check if vis has changed */
        if (0 == ui.discoveryComboBox->currentIndex())
        {
                visState |= RS_VS_DISC_ON;
        }

        if (visState != detail.visState)
        {
                rsPeers->setVisState(ownId, visState);
        }

	/*if (0 != netIndex)
	{
		saveAddr = true;
	}*/

	/*if (saveAddr)
	{
	  rsPeers->setLocalAddress(rsPeers->getOwnId(), ui.localAddress->text().toStdString(), ui.localPort->value());
	  rsPeers->setExtAddress(rsPeers->getOwnId(), ui.extAddress->text().toStdString(), ui.extPort->value());
	}*/

	rsicontrol->ConfigSetDataRates( ui.doubleSpinBoxDownloadSpeed->value(), ui.doubleSpinBoxUploadSpeed->value() );
	loadNetwork();
}

//void QuickStartWizard::toggleTunnelConnection(bool b)
//{
//        std::cerr << "QuickStartWizard::toggleTunnelConnection() set tunnel to : " << b << std::endl;
//        rsPeers->allowTunnelConnection(b) ;
//}
