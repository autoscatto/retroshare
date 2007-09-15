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


#include "rshare.h"
#include "SearchDialog.h"
#include "rsiface/rsiface.h"

#include <iostream>
#include <sstream>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>

/* Images for context menu icons */
#define IMAGE_START       ":/images/start.png"

/** Constructor */
SearchDialog::SearchDialog(QWidget *parent)
: MainPage(parent), nextSearchId(1)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.searchResultWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( searchtableWidgetCostumPopupMenu( QPoint ) ) );

  connect( ui.searchSummaryWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( searchtableWidget2CostumPopupMenu( QPoint ) ) );

  connect( ui.lineEdit, SIGNAL( returnPressed ( void ) ), this, SLOT( searchKeywords( void ) ) );
  connect( ui.pushButtonsearch, SIGNAL( released ( void ) ), this, SLOT( searchKeywords( void ) ) );
  //connect( ui.searchSummaryWidget, SIGNAL( itemSelectionChanged ( void ) ), this, SLOT( selectSearchResults( void ) ) );

  connect ( ui.searchSummaryWidget, SIGNAL( currentItemChanged ( QTreeWidgetItem *, QTreeWidgetItem * ) ),
  		this, SLOT( selectSearchResults( void ) ) );

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void SearchDialog::searchtableWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      downloadAct = new QAction(QIcon(IMAGE_START), tr( "Download" ), this );
      connect( downloadAct , SIGNAL( triggered() ), this, SLOT( download() ) );
      
      broadcastonchannelAct = new QAction( tr( "Broadcast on Channel" ), this );
      connect( broadcastonchannelAct , SIGNAL( triggered() ), this, SLOT( broadcastonchannel() ) );
      
      recommendtofriendsAct = new QAction( tr( "Recommend to Friends" ), this );
      connect( recommendtofriendsAct , SIGNAL( triggered() ), this, SLOT( recommendtofriends() ) );


      contextMnu.clear();
      contextMnu.addAction( downloadAct);
      contextMnu.addAction( broadcastonchannelAct);
      contextMnu.addAction( recommendtofriendsAct);
      contextMnu.exec( mevent->globalPos() );
}


void SearchDialog::download()
{


}


void SearchDialog::broadcastonchannel()
{


}


void SearchDialog::recommendtofriends()
{
   
   
}


/** context menu searchTablewidget2 **/
void SearchDialog::searchtableWidget2CostumPopupMenu( QPoint point )
{

      QMenu contextMnu2( this );
      QMouseEvent *mevent2 = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      searchremoveAct = new QAction( tr( "Remove" ), this );
      connect( searchremoveAct , SIGNAL( triggered() ), this, SLOT( searchremove() ) );
      
      searchremoveallAct = new QAction( tr( "Remove All" ), this );
      connect( searchremoveallAct , SIGNAL( triggered() ), this, SLOT( searchremoveall() ) );
      

      contextMnu2.clear();
      contextMnu2.addAction( searchremoveAct);
      contextMnu2.addAction( searchremoveallAct);
      contextMnu2.exec( mevent2->globalPos() );
}

/** remove selected search result **/
void SearchDialog::searchremove()
{


}

/** remove all search results **/
void SearchDialog::searchremoveall()
{
   
   
}


void SearchDialog::searchKeywords()
{

	std::string txt = (ui.lineEdit->text()).toStdString();

	std::cerr << "SearchDialog::searchKeywords() : " << txt;
	std::cerr << std::endl;

	/* extract keywords from lineEdit */
	std::list<std::string> words;
	
	words.push_back(txt);

	if (words.size() < 1)
	{
		/* ignore */
		return;
	}

	/* call to core */
	std::list<FileDetail> results;
	rsicontrol -> SearchKeywords(words, results);

	/* translate search results */
	int searchId = nextSearchId++;
	std::ostringstream out;
	out << searchId;

	std::list<FileDetail>::iterator it;
	for(it = results.begin(); it != results.end(); it++)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem();
		item->setText(0, QString::fromStdString(it->name));
		item->setText(1, QString::fromStdString(it->hash));
		item->setText(2, QString::fromStdString(it->id));
		item->setText(3, QString::fromStdString(out.str()));
		/*
		 *
		 */
		ui.searchResultWidget->addTopLevelItem(item);
	}

	/* add to the summary as well */

	QTreeWidgetItem *item = new QTreeWidgetItem();
	item->setText(0, QString::fromStdString(txt));
	std::ostringstream out2;
	out2 << results.size();

	item->setText(1, QString::fromStdString(out2.str()));
	item->setText(2, QString::fromStdString(out.str()));

	ui.searchSummaryWidget->addTopLevelItem(item);
	ui.searchSummaryWidget->setCurrentItem(item);

	/* select this search result */
	selectSearchResults();
}


//void QTreeWidget::currentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous )  [signal]


void SearchDialog::selectSearchResults()
{
	/* highlight this search in summary window */
	QTreeWidgetItem *ci = ui.searchSummaryWidget->currentItem();

	/* get the searchId text */
	QString searchId = ci->text(2);

	std::cerr << "SearchDialog::selectSearchResults(): searchId: " << searchId.toStdString();
	std::cerr << std::endl;

	/* show only matching searchIds in main window */
	int items = ui.searchResultWidget->topLevelItemCount();
	for(int i = 0; i < items; i++)
	{
		/* get item */
		QTreeWidgetItem *ti = ui.searchResultWidget->topLevelItem(i);
		if (ti->text(3) == searchId)
		{
			ti->setHidden(false);
		}
		else
		{
			ti->setHidden(true);
		}
	}
	ui.searchResultWidget->update();
}


