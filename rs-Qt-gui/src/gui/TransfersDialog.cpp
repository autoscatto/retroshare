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


#include "rshare.h"
#include "TransfersDialog.h"
#include "moreinfo/moreinfo.h"
#include <QHeaderView>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>

#include <sstream>
#include "rsiface/rsiface.h"

/* Images for context menu icons */
#define IMAGE_INFO                 ":/images/fileinfo.png"
#define IMAGE_CANCEL               ":/images/delete.png"
#define IMAGE_CLEARCOMPLETED       ":/images/deleteall.png"

/** Constructor */
TransfersDialog::TransfersDialog(QWidget *parent)
: MainPage(parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.downtreeWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( downtreeWidgetCostumPopupMenu( QPoint ) ) );

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void TransfersDialog::downtreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

//      showdowninfoAct = new QAction(QIcon(IMAGE_INFO), tr( "Details..." ), this );
//      connect( showdowninfoAct , SIGNAL( triggered() ), this, SLOT( showDownInfoWindow() ) );

	  cancelAct = new QAction(QIcon(IMAGE_CANCEL), tr( "Cancel" ), this );
      connect( cancelAct , SIGNAL( triggered() ), this, SLOT( cancel() ) );
      
      clearcompletedAct = new QAction(QIcon(IMAGE_CLEARCOMPLETED), tr( "Clear Completed" ), this );
      connect( clearcompletedAct , SIGNAL( triggered() ), this, SLOT( clearcompleted() ) );

      contextMnu.clear();
      contextMnu.addAction( cancelAct);
//      contextMnu.addSeparator();     
//      contextMnu.addAction( showdowninfoAct);
      contextMnu.addSeparator();
      contextMnu.addAction( clearcompletedAct);
      contextMnu.exec( mevent->globalPos() );
}

/** Shows Downloads Informations */
void TransfersDialog::showDownInfoWindow()
{
    moreinfo *detailsdlg = new moreinfo();
    detailsdlg->show();
} 


	/* get the list of Transfers from the RsIface.  */
void TransfersDialog::insertTransfers()
{
	rsiface->lockData(); /* Lock Interface */

	std::list<FileTransferInfo>::const_iterator it;
	const std::list<FileTransferInfo> &transfers = rsiface->getTransferList();

	/* get a link to the table */
        QTreeWidget *transferWidget = ui.downtreeWidget;

	/* remove old items ??? */

        QList<QTreeWidgetItem *> items;
	for(it = transfers.begin(); it != transfers.end(); it++)
	{
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		item -> setText(0, QString::fromStdString(it->fname));
		item -> setText(1, QString::fromStdString(it->source));
		switch(it->downloadStatus)
		{
			/* XXX HAND CODED! */
			case 0: /* FAILED */
				item -> setText(2, "Failed");
				break;
			case 1: /* OKAY */
				item -> setText(2, "Okay");
				break;
			case 2: /* COMPLETE */
			default:
				item -> setText(2, "Complete");
				break;
		}
		{
			std::ostringstream out;
			out << it -> size;
			item -> setText(3, QString::fromStdString(out.str()));
		}
		{
			std::ostringstream out;
			out << it -> tfRate << " kB/s";
			item -> setText(4, QString::fromStdString(out.str()));
		}
		{
			std::ostringstream out;
			out << (it -> transfered * 100.0 / it -> size) << "%";
			item -> setText(5, QString::fromStdString(out.str()));
		}
		{
			std::ostringstream out;
			out << it -> transfered << "/" << it -> size << " Bytes";
			item -> setText(6, QString::fromStdString(out.str()));
		}
		{
			std::ostringstream out;
			out << it -> id;
			item -> setText(7, QString::fromStdString(out.str()));
		}

		/* add to the list */
		items.append(item);
	}

	/* add the items in! */
	transferWidget->clear();
	transferWidget->insertTopLevelItems(0, items);

	rsiface->unlockData(); /* UnLock Interface */
}

/* Utility Fns */
std::string getFileRsCertId(QTreeWidgetItem *i)
{
	std::string id = (i -> text(7)).toStdString();
	return id;
}

std::string getFileName(QTreeWidgetItem *i)
{
	std::string id = (i -> text(0)).toStdString();
	return id;
}

/** Open a QFileDialog to browse for export a file. */
void TransfersDialog::cancel()
{
        QTreeWidgetItem *c = getCurrentPeer();
        std::cerr << "TransfersDialog::cancel()" << std::endl;
	if (!c)
	{
        	std::cerr << "TransfersDialog::cancel()";
        	std::cerr << "Noone Selected -- sorry" << std::endl;
		return;
	}

	std::string id = getFileRsCertId(c);
	std::string file = getFileName(c);
	if (file != "")
	{
        	std::cerr << "TransfersDialog::cancel(): " << file << std::endl;
        	std::cerr << std::endl;
		rsicontrol->FileCancel(id, file);
	}
}

void TransfersDialog::clearcompleted()
{
        std::cerr << "TransfersDialog::clearcompleted()" << std::endl;
	rsicontrol->FileClearCompleted();
}


QTreeWidgetItem *TransfersDialog::getCurrentPeer()
{
	/* get the current, and extract the Id */

	/* get a link to the table */
        QTreeWidget *peerWidget = ui.downtreeWidget;
        QTreeWidgetItem *item = peerWidget -> currentItem();
        if (!item)
        {
		std::cerr << "Invalid Current Item" << std::endl;
		return NULL;
	}

	/* Display the columns of this item. */
	std::ostringstream out;
        out << "CurrentPeerItem: " << std::endl;

	for(int i = 0; i < 5; i++)
	{
		QString txt = item -> text(i);
		out << "\t" << i << ":" << txt.toStdString() << std::endl;
	}
	std::cerr << out.str();
	return item;
}



