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

#include <QFile>
#include <QFileInfo>

#include "rshare.h"
#include "common/vmessagebox.h"
#include "ConnectionsDialog.h"
#include "connect/ConnectDialog.h"
#include "authdlg/AuthorizationDialog.h"
#include "rsiface/rsiface.h"
#include <sstream>


#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>

/* Images for context menu icons */
#define IMAGE_LOADCERT       ":/images/loadcert16.png"
#define IMAGE_PEERDETAILS    ":/images/peerdetails_16x16.png"
#define IMAGE_AUTH           ":/images/encrypted16.png"

RsCertId getNeighRsCertId(QTreeWidgetItem *i);

/** Constructor */
ConnectionsDialog::ConnectionsDialog(QWidget *parent)
: MainPage(parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.connecttreeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( connecttreeWidgetCostumPopupMenu( QPoint ) ) );


  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void ConnectionsDialog::connecttreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      peerdetailsAct = new QAction(QIcon(IMAGE_PEERDETAILS), tr( "Peer Details" ), this );
      connect( peerdetailsAct , SIGNAL( triggered() ), this, SLOT( peerdetails() ) );
      
      authAct = new QAction(QIcon(IMAGE_AUTH), tr( "Authenticate" ), this );
      connect( authAct , SIGNAL( triggered() ), this, SLOT( showAuthDialog() ) );
      
      loadcertAct = new QAction(QIcon(IMAGE_LOADCERT), tr( "Load Certificate" ), this );
      connect( loadcertAct , SIGNAL( triggered() ), this, SLOT( loadneighbour() ) );


      contextMnu.clear();
      contextMnu.addAction( peerdetailsAct);
      contextMnu.addAction( authAct);
      contextMnu.addAction( loadcertAct);
      contextMnu.exec( mevent->globalPos() );
}

/** Shows Peer Informations */
void ConnectionsDialog::peerdetails()
{
    static ConnectDialog *connectdialog = new ConnectDialog();
/* fill it in */
    std::cerr << "ConnectionsDialog::peerdetails()" << std::endl;

    QTreeWidgetItem *wi = getCurrentNeighbour();
    if (!wi)
    	return;

    RsCertId id = getNeighRsCertId(wi);

    /* grab the interface and check person */
    rsiface->lockData(); /* Lock Interface */

    std::map<RsCertId,NeighbourInfo>::const_iterator it;
    const std::map<RsCertId,NeighbourInfo> &neighs = 
				rsiface->getNeighbourMap();

    it = neighs.find(id);
    if (it == neighs.end())
    {
    	return;
    }

    connectdialog -> setInfo(it->second.name, it->second.trustString, it->second.org, 
    		it->second.loc, it->second.country, "Signers");

    rsiface->unlockData(); /* UnLock Interface */

    connectdialog->show();
}

/** Shows Connect Dialog */
void ConnectionsDialog::showAuthDialog()
{
    static AuthorizationDialog *authorizationdialog = new AuthorizationDialog();
    QTreeWidgetItem *wi = getCurrentNeighbour();
    if (!wi)
    	return;

    RsCertId id = getNeighRsCertId(wi);
    std::ostringstream out;
    out << id;
    authorizationdialog->setAuthCode(out.str(), wi->text(9).toStdString());
    authorizationdialog->show();
}




/** Open a QFileDialog to browse for a pem/pqi file. */
void ConnectionsDialog::loadcert()
{
  /* Create a new input dialog, which allows users to create files, too */
  QFileDialog *dialog = new QFileDialog(this, tr("Select a pem/pqi File"));
  //dialog->setDirectory(QFileInfo(ui.lineTorConfig->text()).absoluteDir());
  //dialog->selectFile(QFileInfo(ui.lineTorConfig->text()).fileName());
  dialog->setFileMode(QFileDialog::AnyFile);
  dialog->setReadOnly(false);

  /* Prompt the user to select a file or create a new one */
  if (!dialog->exec() || dialog->selectedFiles().isEmpty()) {
    return;
  }
  QString filename = QDir::convertSeparators(dialog->selectedFiles().at(0));
 
  /* Check if the file exists */
  QFile torrcFile(filename);
  if (!QFileInfo(filename).exists()) {
    /* The given file does not exist. Should we create it? */
    int response = VMessageBox::question(this,
                     tr("File Not Found"),
                     tr("%1 does not exist. Would you like to create it?")
                                                            .arg(filename),
                     VMessageBox::Yes, VMessageBox::No);
    
    if (response == VMessageBox::No) {
      /* Don't create it. Just bail. */
      return;
    }
    /* Attempt to create the specified file */
    if (!torrcFile.open(QIODevice::WriteOnly)) {
      VMessageBox::warning(this,
        tr("Failed to Create File"),
        tr("Unable to create %1 [%2]").arg(filename)
                                      .arg(torrcFile.errorString()),
        VMessageBox::Ok);
      return;
    }
  }
  //ui.lineTorConfig->setText(filename);
}



#include <sstream>

/* get the list of Neighbours from the RsIface.  */
void ConnectionsDialog::insertConnect()
{
        rsiface->lockData(); /* Lock Interface */

        std::map<RsCertId,NeighbourInfo>::const_iterator it;
        const std::map<RsCertId,NeighbourInfo> &neighs = 
				rsiface->getNeighbourMap();

	/* get a link to the table */
        QTreeWidget *connectWidget = ui.connecttreeWidget;

	/* remove old items ??? */
	connectWidget->clear();
	connectWidget->setColumnCount(7);

        QList<QTreeWidgetItem *> items;
	for(it = neighs.begin(); it != neighs.end(); it++)
	{
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */

		/* (0) Accept/Deny */
                item -> setText(0, QString::fromStdString(it->second.acceptString));
		/* (1) Trust Level */
                item -> setText(1, QString::fromStdString(it->second.trustString));
		/* (2) Last Connect */
                item -> setText(2, QString::fromStdString(it->second.lastConnect));
                /* (3) Person */
		item -> setText(3, QString::fromStdString(it->second.name));

		/* Others */
		item -> setText(4, QString::fromStdString(it->second.org));
		item -> setText(5, QString::fromStdString(it->second.loc));
		item -> setText(6, QString::fromStdString(it->second.country));

                /* (4) Peer Address */
                item -> setText(7, QString::fromStdString(it->second.peerAddress));
		{
			std::ostringstream out;
			out << it -> second.id;
			item -> setText(8, QString::fromStdString(out.str()));
		}
		item -> setText(9, QString::fromStdString(it->second.authCode));

		/* add to the list */
		items.append(item);
	}

	/* add the items in! */
	connectWidget->insertTopLevelItems(0, items);

	rsiface->unlockData(); /* UnLock Interface */

	connectWidget->update(); /* update display */

}

QTreeWidgetItem *ConnectionsDialog::getCurrentNeighbour()
{ 
        /* get the current, and extract the Id */
  
        /* get a link to the table */
        QTreeWidget *connectWidget = ui.connecttreeWidget;
        QTreeWidgetItem *item = connectWidget -> currentItem();
        if (!item) 
        {
                std::cerr << "Invalid Current Item" << std::endl;
                return NULL;
        }
    
        /* Display the columns of this item. */
        std::ostringstream out;
        out << "CurrentNeighbourItem: " << std::endl;

        for(int i = 0; i < 5; i++)
        {
                QString txt = item -> text(i);
                out << "\t" << i << ":" << txt.toStdString() << std::endl;
        }
        std::cerr << out.str();
        return item;
}   

/* Utility Fns */
RsCertId getNeighRsCertId(QTreeWidgetItem *i)
{
        RsCertId id = (i -> text(8)).toStdString();
        return id;
}   
  
/* So from the Neighbours Dialog we can call the following control Functions:
 * (1) Load Certificate             NeighLoadCertificate(std::string file)
 * (2) Neigh  Auth                  NeighAuthFriend(id, code)
 * (3) Neigh  Add                   NeighAddFriend(id)
 *
 * All of these rely on the finding of the current Id.
 */

void ConnectionsDialog::loadneighbour()
{
        std::cerr << "ConnectionsDialog::loadneighbour()" << std::endl;
        QString fileName = QFileDialog::getOpenFileName(this, tr("Select Certificate"), "",
	                                             tr("Certificates (*.pqi *.pem)"));

	std::string file = fileName.toStdString();
	if (file != "")
	{
        	rsicontrol->NeighLoadCertificate(file);
	}
}

void ConnectionsDialog::addneighbour()
{
        QTreeWidgetItem *c = getCurrentNeighbour();
        std::cerr << "ConnectionsDialog::addneighbour()" << std::endl;
        /*
        rsServer->NeighAddFriend(getNeighRsCertId(c));
        */
}

void ConnectionsDialog::authneighbour()
{
        QTreeWidgetItem *c = getCurrentNeighbour();
        std::cerr << "ConnectionsDialog::authneighbour()" << std::endl;
        /*
	RsAuthId code;
        rsServer->NeighAuthFriend(getNeighRsCertId(c), code);
        */
}

