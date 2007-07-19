/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006,2007 crypton
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
#include "DLListDelegate.h"
#include "ULListDelegate.h"

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QHeaderView>
#include <QStandardItemModel>

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

    connect( ui.downloadList, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( downloadListCostumPopupMenu( QPoint ) ) );

    // Set Download list model
    DLListModel = new QStandardItemModel(0,8);
    DLListModel->setHeaderData(NAME, Qt::Horizontal, tr("Name", "i.e: file name"));
    DLListModel->setHeaderData(SIZE, Qt::Horizontal, tr("Size", "i.e: file size"));
    DLListModel->setHeaderData(PROGRESS, Qt::Horizontal, tr("Progress", "i.e: % downloaded"));
    DLListModel->setHeaderData(DLSPEED, Qt::Horizontal, tr("Speed", "i.e: Download speed"));
    DLListModel->setHeaderData(SOURCES, Qt::Horizontal, tr("Sources", "i.e: Sources"));
    DLListModel->setHeaderData(STATUS, Qt::Horizontal, tr("Status"));
    DLListModel->setHeaderData(COMPLETED, Qt::Horizontal, tr("Completed", ""));
    DLListModel->setHeaderData(REMAINING, Qt::Horizontal, tr("Remaining", "i.e: Estimated Time of Arrival / Time left"));
    ui.downloadList->setModel(DLListModel);
    DLDelegate = new DLListDelegate();
    ui.downloadList->setItemDelegate(DLDelegate);
  
  	//Selection Setup
	selection = ui.downloadList->selectionModel();
  
    /* Set header resize modes and initial section sizes Downloads TreeView*/
	QHeaderView * _header = ui.downloadList->header () ;
   	_header->setResizeMode (0, QHeaderView::Interactive);
	_header->setResizeMode (1, QHeaderView::Interactive);
	_header->setResizeMode (2, QHeaderView::Interactive);
	_header->setResizeMode (3, QHeaderView::Interactive);
	_header->setResizeMode (4, QHeaderView::Interactive);
	_header->setResizeMode (5, QHeaderView::Interactive);
	_header->setResizeMode (6, QHeaderView::Interactive);
	_header->setResizeMode (7, QHeaderView::Interactive);

    
	_header->resizeSection ( 0, 100 );
	_header->resizeSection ( 1, 100 );
	_header->resizeSection ( 2, 170 );
	_header->resizeSection ( 3, 100 );
	_header->resizeSection ( 4, 100 );
	_header->resizeSection ( 5, 100 );
	_header->resizeSection ( 6, 100 );
	_header->resizeSection ( 7, 100 );
	
	// Set Upload list model
    ULListModel = new QStandardItemModel(0,6);
    ULListModel->setHeaderData(UNAME, Qt::Horizontal, tr("Name", "i.e: file name"));
    ULListModel->setHeaderData(USIZE, Qt::Horizontal, tr("Size", "i.e: file size"));
    ULListModel->setHeaderData(USERNAME, Qt::Horizontal, tr("User Name", "i.e: user name"));
    ULListModel->setHeaderData(UPROGRESS, Qt::Horizontal, tr("Progress", "i.e: % uploaded"));
    ULListModel->setHeaderData(ULSPEED, Qt::Horizontal, tr("Speed", "i.e: upload speed"));
    ULListModel->setHeaderData(USTATUS, Qt::Horizontal, tr("Status"));
    ULListModel->setHeaderData(UTRANSFERRED, Qt::Horizontal, tr("Transferred", ""));
    ui.uploadsList->setModel(ULListModel);
    ULDelegate = new ULListDelegate();
    ui.uploadsList->setItemDelegate(ULDelegate);
  
  	//Selection Setup
	selection = ui.uploadsList->selectionModel();
	
	/* Set header resize modes and initial section sizes Uploads TreeView*/
	QHeaderView * upheader = ui.uploadsList->header () ;
	upheader->setResizeMode (0, QHeaderView::Interactive);
	upheader->setResizeMode (1, QHeaderView::Interactive);
	upheader->setResizeMode (2, QHeaderView::Interactive);
	upheader->setResizeMode (3, QHeaderView::Interactive);
	upheader->setResizeMode (4, QHeaderView::Interactive);
	upheader->setResizeMode (5, QHeaderView::Interactive);
	upheader->setResizeMode (6, QHeaderView::Interactive);
    
	upheader->resizeSection ( 0, 100 );
	upheader->resizeSection ( 1, 100 );
	upheader->resizeSection ( 2, 100 );
	upheader->resizeSection ( 3, 100 );
	upheader->resizeSection ( 4, 100 );
	upheader->resizeSection ( 5, 100 );
	upheader->resizeSection ( 6, 100 );

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void TransfersDialog::downloadListCostumPopupMenu( QPoint point )
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

void TransfersDialog::updateProgress(int value)
{
	for(int i = 0; i <= DLListModel->rowCount(); i++) {
		if(selection->isRowSelected(i, QModelIndex())) {
			editItem(i, PROGRESS, QVariant((double)value));
		}
	}
}

TransfersDialog::~TransfersDialog()
{
	;
}



int TransfersDialog::addItem(QString symbol, QString name, qlonglong fileSize, double progress, double dlspeed, QString sources,  QString status, qlonglong completed, qlonglong remaining)
{
	int row;
	QString sl;
	//QIcon icon(symbol);
	name.insert(0, " ");
	//sl.sprintf("%d / %d", seeds, leechs);
	row = DLListModel->rowCount();
	DLListModel->insertRow(row);

	//DLListModel->setData(DLListModel->index(row, NAME), QVariant((QIcon)icon), Qt::DecorationRole);
	DLListModel->setData(DLListModel->index(row, NAME), QVariant((QString)name), Qt::DisplayRole);
	DLListModel->setData(DLListModel->index(row, SIZE), QVariant((qlonglong)fileSize));
	DLListModel->setData(DLListModel->index(row, PROGRESS), QVariant((double)progress));
	DLListModel->setData(DLListModel->index(row, DLSPEED), QVariant((double)dlspeed));
	DLListModel->setData(DLListModel->index(row, SOURCES), QVariant((QString)sources));
	DLListModel->setData(DLListModel->index(row, STATUS), QVariant((QString)status));
	DLListModel->setData(DLListModel->index(row, COMPLETED), QVariant((qlonglong)completed));
	DLListModel->setData(DLListModel->index(row, REMAINING), QVariant((qlonglong)remaining));
	return row;
}

void TransfersDialog::delItem(int row)
{
	DLListModel->removeRow(row, QModelIndex());
}

void TransfersDialog::editItem(int row, int column, QVariant data)
{
	//QIcon *icon;
	switch(column) {
		//case SYMBOL:
		//	icon = new QIcon(data.toString());
		//	DLListModel->setData(DLListModel->index(row, NAME), QVariant((QIcon)*icon), Qt::DecorationRole);
		//	delete icon;
		//	break;
		case NAME:
			DLListModel->setData(DLListModel->index(row, NAME), data, Qt::DisplayRole);
			break;
		case SIZE:
			DLListModel->setData(DLListModel->index(row, SIZE), data);
			break;
		case PROGRESS:
			DLListModel->setData(DLListModel->index(row, PROGRESS), data);
			break;
		case DLSPEED:
			DLListModel->setData(DLListModel->index(row, DLSPEED), data);
			break;
		case SOURCES:
			DLListModel->setData(DLListModel->index(row, SOURCES), data);
			break;
		case STATUS:
			DLListModel->setData(DLListModel->index(row, STATUS), data);
			break;
		case COMPLETED:
			DLListModel->setData(DLListModel->index(row, COMPLETED), data);
			break;
		case REMAINING:
			DLListModel->setData(DLListModel->index(row, REMAINING), data);
			break;
	}
}

	/* get the list of Transfers from the RsIface.  **/
void TransfersDialog::insertTransfers()
{
	QString symbol, name, sources, status;
	qlonglong fileSize, completed, remaining;
	double progress, dlspeed; 

	//remove all Items 
	for(int i = DLListModel->rowCount(); i >= 0; i--) 
	{
		delItem(i);
	}
	
	
	//nun aktuelle DownloadListe hinzufügen
	rsiface->lockData(); /* Lock Interface */

	std::list<FileTransferInfo>::const_iterator it;
	const std::list<FileTransferInfo> &transfers = rsiface->getTransferList();

	for(it = transfers.begin(); it != transfers.end(); it++) 
	{
		
		symbol  	= "";
		name    	= QString::fromStdString(it->fname);
		sources  	= QString::fromStdString(it->source);
		
		switch(it->downloadStatus) 
		{
			/* XXX HAND CODED! */
			case 0: /* FAILED */
				status = "Failed";
				break;
			case 1: /* Downloading */
				status = "Downloading";
				break;
		    case 2: /* COMPLETE */
			default:
				status = "Complete";
				break;
		
        }
        
		dlspeed  	= it->tfRate * 1024.0;
		fileSize 	= it->size;
		completed 	= it->transfered;
		progress 	= it->transfered * 100.0 / it->size;
		remaining   = (it->size - it->transfered) / (it->tfRate * 1024.0);

		
		//?? 
		std::ostringstream out;
		out << it -> id;
		
		addItem(symbol, name, fileSize, progress, dlspeed, sources,  status, completed, remaining);
	}

	rsiface->unlockData(); /* UnLock Interface */
}

/* Utility Fns */
//std::string getFileRsCertId(QTreeWidgetItem *i)
//{
//	std::string id = (i -> text(7)).toStdString();
//	return id;
//}

//std::string getFileName(QTreeWidgetItem *i)
//{
//	std::string id = (i -> text(0)).toStdString();
//	return id;
//}

/** Open a QFileDialog to browse for export a file. */
void TransfersDialog::cancel()
{
    /*    DLListModel *c = getCurrentPeer();
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
	}*/
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
    /*    QTreeWidget *peerWidget = ui.downloadList;
        QTreeWidgetItem *item = peerWidget -> currentItem();
        if (!item)
        {
		std::cerr << "Invalid Current Item" << std::endl;
		return NULL;
	}*/

	/* Display the columns of this item. */
	/* std::ostringstream out;
        out << "CurrentPeerItem: " << std::endl;

	for(int i = 0; i < 5; i++)
	{
		QString txt = item -> text(i);
		out << "\t" << i << ":" << txt.toStdString() << std::endl;
	}
	std::cerr << out.str();
	return item;
	*/
}



