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
#include "SharedFilesDialog.h"
#include "rsiface/rsiface.h"
#include "rsiface/RemoteDirModel.h"

#include <iostream>
#include <sstream>


#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>

/* Images for context menu icons */
#define IMAGE_DOWNLOAD       ":/images/start.png"

/** Constructor */
SharedFilesDialog::SharedFilesDialog(QWidget *parent)
: MainPage(parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.localDirTreeView, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( shareddirtreeWidgetCostumPopupMenu( QPoint ) ) );

  connect( ui.remoteDirTreeView, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( shareddirtreeviewCostumPopupMenu( QPoint ) ) );

/*
  connect( ui.remoteDirTreeView, SIGNAL( itemExpanded( QTreeWidgetItem * ) ), 
	this, SLOT( checkForLocalDirRequest( QTreeWidgetItem * ) ) );

  connect( ui.localDirTreeWidget, SIGNAL( itemExpanded( QTreeWidgetItem * ) ), 
	this, SLOT( checkForRemoteDirRequest( QTreeWidgetItem * ) ) );
*/


  model = new RemoteDirModel(true);
  localModel = new RemoteDirModel(false);
  ui.remoteDirTreeView->setModel(model);
  ui.localDirTreeView->setModel(localModel);

  connect( ui.remoteDirTreeView, SIGNAL( collapsed(const QModelIndex & ) ),
  	model, SLOT(  collapsed(const QModelIndex & ) ) );
  connect( ui.remoteDirTreeView, SIGNAL( expanded(const QModelIndex & ) ),
  	model, SLOT(  expanded(const QModelIndex & ) ) );

  connect( ui.localDirTreeView, SIGNAL( collapsed(const QModelIndex & ) ),
  	localModel, SLOT(  collapsed(const QModelIndex & ) ) );
  connect( ui.localDirTreeView, SIGNAL( expanded(const QModelIndex & ) ),
  	localModel, SLOT(  expanded(const QModelIndex & ) ) );

  /* 
  ui.remoteDirTreeView->setRootIndex(model->index(QDir::currentPath()));
  */

  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void SharedFilesDialog::shareddirtreeviewCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      downloadAct = new QAction(QIcon(IMAGE_DOWNLOAD), tr( "Download" ), this );
      connect( downloadAct , SIGNAL( triggered() ), this, SLOT( downloadRemoteSelected() ) );
      
    //  addMsgAct = new QAction( tr( "Add to Message" ), this );
    //  connect( addMsgAct , SIGNAL( triggered() ), this, SLOT( addMsgRemoteSelected() ) );
      

      contextMnu.clear();
      contextMnu.addAction( downloadAct);
   //   contextMnu.addAction( addMsgAct);
      contextMnu.exec( mevent->globalPos() );
}


void SharedFilesDialog::downloadRemoteSelected()
{
  /* call back to the model (which does all the interfacing? */

  std::cerr << "Downloading Files";
  std::cerr << std::endl;

  QItemSelectionModel *qism = ui.remoteDirTreeView->selectionModel();
  model -> downloadSelected(qism->selectedIndexes());

}


void SharedFilesDialog::addMsgRemoteSelected()
{
  /* call back to the model (which does all the interfacing? */

  std::cerr << "Recommending Files";
  std::cerr << std::endl;

  QItemSelectionModel *qism = ui.remoteDirTreeView->selectionModel();
  model -> recommendSelected(qism->selectedIndexes());


}


void SharedFilesDialog::shareddirtreeWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu2( this );
      QMouseEvent *mevent2 = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      openfileAct = new QAction( tr( "Recommend" ), this );
      connect( openfileAct , SIGNAL( triggered() ), this, SLOT( recommendfile() ) );
      
     // openfolderAct = new QAction( tr( "Play File" ), this );
     // connect( openfolderAct , SIGNAL( triggered() ), this, SLOT( openfile() ) );
      

      contextMnu2.clear();
      contextMnu2.addAction( openfileAct);
     // contextMnu2.addAction( openfolderAct);
      contextMnu2.exec( mevent2->globalPos() );
}


void SharedFilesDialog::recommendfile()
{
  /* call back to the model (which does all the interfacing? */

  std::cerr << "Downloading Files";
  std::cerr << std::endl;

  QItemSelectionModel *qism = ui.localDirTreeView->selectionModel();
  localModel -> recommendSelected(qism->selectedIndexes());
}



void SharedFilesDialog::openfile()
{
  /* call back to the model (which does all the interfacing? */

  std::cerr << "Opening File";
  std::cerr << std::endl;

  QItemSelectionModel *qism = ui.localDirTreeView->selectionModel();
  model -> openSelected(qism->selectedIndexes());

}


void SharedFilesDialog::openfolder()
{

}

#ifdef NO_MORE

	/* get the list of peers from the RsIface.  */
void  SharedFilesDialog::insertFiles(bool update_local)
{
	rsiface->lockData();   /* Lock Interface */

	std::list<PersonInfo>::const_iterator it;

	const std::list<PersonInfo> &remote = rsiface->getRemoteDirectoryList();
	const std::list<PersonInfo> &local  = rsiface->getLocalDirectoryList();

	/* get a link to the table */
	const std::list<PersonInfo> *dirs = &remote;
	QTreeWidget *tree = ui.localDirTreeWidget;
	if (update_local)
	{
		/* select the other 
		 * will do (when its a tree).
		tree = ui.localDirTreeWidget;
		dirs = &local;
		 */

		/* drop out */
		rsiface->unlockData();
		return; 
	}

	/* remove old items ??? TODO */
        /* remove old items ??? */
        tree->clear(); 
        /* tree->setColumnCount(5); */

        QList<QTreeWidgetItem *> items;
	for(it = dirs->begin(); it != dirs->end(); it++)
	{
		/* iterate through the people, 
		 * will have to go recursively through
		 * the directories.
		 */

		/* make a widget per person */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* add all the labels */
		item -> setText(0, QString::fromStdString(it->name));
		item -> setText(1, "Entries");
		item -> setText(2, "Total Size");

                /* Hidden ones: */
                {
                        std::ostringstream out;
                        out << it -> id;
                        item -> setText(3, QString::fromStdString(out.str()));
                }


		/* add to the list */
		items.append(item);

		/* add all the directories recursively */
		addDirectories(it->rootdir, item);
	}

	/* add the items in! */
	tree->insertTopLevelItems(0, items);

	rsiface->unlockData(); /* UnLock Interface */

	tree -> update();
}

void SharedFilesDialog::addDirectories(const DirInfo &root, QTreeWidgetItem *parent)
{
	const std::list<DirInfo> &dirs = root.subdirs;
	const std::list<FileInfo> &files = root.files;

	std::list<DirInfo>::const_iterator it;
	std::list<FileInfo>::const_iterator fit;

	int i = 0;

	for(it = dirs.begin(); it != dirs.end(); it++, i++)
	{
		/* make a widget per person */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		item -> setText(0, QString::fromStdString(it->dirname));
		std::ostringstream str1, str2;
		str1 << it->nofiles; /* No Entries */
		str2 << it->nobytes; /* Total Bytes */

		item -> setText(1, QString::fromStdString(str1.str()));
		item -> setText(2, QString::fromStdString(str2.str()));

		/* add all the directories recursively */
		addDirectories(*it, item);

		/* add to parent?? */
		// items.append(item);
	}


	for(fit = files.begin(); fit != files.end(); fit++, i++)
	{
		/* make a widget per person */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		item -> setText(0, QString::fromStdString(fit->fname));
		std::ostringstream str1, str2;
		str1 << fit->size;  
		str2 << fit->rank;

		item -> setText(1, QString::fromStdString(str1.str()));
		item -> setText(2, QString::fromStdString(str2.str()));

		/* add all the directories recursively */

		/* add to parent?? */
		//items.append(item);
	}

	if (!i)
	{
	  if (root.nofiles > 0)
	  {
		/* Add a <Empty> item */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		//item -> setText(0, QString::fromStdString(fit->fname));
		item -> setText(0, "<Loading ...>");
	  }
	  else
	  {
		/* Add a <Empty> item */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		item -> setText(0, "<Empty>");
	  }
	}
		
	return;
}


void SharedFilesDialog::checkForRemoteDirRequest( QTreeWidgetItem *item )
{
	/* print out a message */
	std::cerr << "SharedFilesDialog::checkForRemoteDirRequest()" << std::endl;

	/* work out the full path (by iterating through the parents) */

	std::string path;
	while(item->parent() != NULL)
	{
		path = std::string("/") 
			+ (item -> text(0)).toStdString() + path;

		item = item -> parent();
		std::cerr << "Path Construction: " << path << std::endl;
	}

	/* extract the RsCertId */
	std::string person = (item -> text(0)).toStdString();
	std::string id = (item -> text(3)).toStdString();

	std::cerr << "Directory Request(" << person << ":" << id << ") : ";
	std::cerr << path << std::endl;

	rsicontrol -> DirectoryRequestUpdate(id, path);

	return;
}

void SharedFilesDialog::checkForLocalDirRequest( QTreeWidgetItem *item )
{
	/* print out a message */
	std::cerr << "SharedFilesDialog::checkForLocalDirRequest()" << std::endl;

	/* work out the full path (by iterating through the parents) */

	std::string path;
	while(item->parent() != NULL)
	{
		path = std::string("/") 
			+ (item -> text(0)).toStdString() + path;

		item = item -> parent();
		std::cerr << "Path Construction: " << path << std::endl;
	}

	/* extract the RsCertId */
	std::string person = (item -> text(0)).toStdString();
	std::string id = (item -> text(3)).toStdString();

	std::cerr << "Directory Request(" << person << ":" << id << ") : ";
	std::cerr << path << std::endl;

	rsicontrol -> DirectoryRequestUpdate(id, path);
	return;
}



	/* Syncronise the Directories with the RsIface.  */
void  SharedFilesDialog::syncDirectories(bool update_local)
{
	rsiface->lockData();   /* Lock Interface */

	std::list<PersonInfo>::const_iterator it, eit;

	const std::list<PersonInfo> &remote = rsiface->getRemoteDirectoryList();
	const std::list<PersonInfo> &local  = rsiface->getLocalDirectoryList();

	/* get a link to the table */
	const std::list<PersonInfo> *dirs = &remote;
	QTreeWidget *tree = ui.localDirTreeWidget;
	if (update_local)
	{
		/* select the other 
		 * will do (when its a tree).
		tree = ui.localDirTreeWidget;
		dirs = &local;
		 */

		/* drop out */
		rsiface->unlockData();
		return; 
	}


	it  = dirs->begin();
	eit = dirs->end();
	
	/* tree->clear(); */
	tree->sortItems(0,Qt::AscendingOrder);

	int i;
	int iTotal = tree->topLevelItemCount();
	for(i = 0; it != eit;)
	{
		/* get both entries */
		QTreeWidgetItem *item = NULL;
		std::string itemId, id;
		std::string name;

		std::cerr << "syncPersonEntry: " << it->name;
		std::cerr << std::endl;

		{
                        std::ostringstream out;
                        out << it -> id;
			id = out.str();
		}

		if (i < iTotal)
		{	
			item = tree->topLevelItem(i);
			name = (item -> text(0)).toStdString();
			itemId = (item -> text(3)).toStdString();

			std::cerr << "syncPersonEntry: Item: " << name;
			std::cerr << "-" << itemId;
			std::cerr << std::endl;

		}				

		/* ok case first */
		if ((item) && (itemId == id))
		{
			std::cerr << "syncPersonEntry: Okay";
			std::cerr << std::endl;

			/* increment both */
			syncDirectory(it->rootdir, item);
			it++;
			i++;
			continue;
		}

		/* remove item */
		if ((item) && (itemId != id))
		{
			std::cerr << "syncPersonEntry: Remove";
			std::cerr << std::endl;

           		QTreeWidgetItem *oitem = tree->takeTopLevelItem(i);
			if (oitem)
				delete oitem;
			iTotal--;
			continue;	
		}

		std::cerr << "syncPersonEntry: Adding";
		std::cerr << std::endl;

		/* if we get here then we are adding it in */
           	QTreeWidgetItem *nitem = new QTreeWidgetItem();

		/* add all the labels */
		nitem -> setText(0, QString::fromStdString(it->name));
		nitem -> setText(1, "Entries");
		nitem -> setText(2, "Total Size");
                /* Hidden ones: */
                {
                        std::ostringstream out;
                        out << it -> id;
                        nitem -> setText(3, QString::fromStdString(out.str()));
                }

		tree->insertTopLevelItem(i, nitem);
			
		std::cerr << "syncPersonEntry: Sync SubDir";
		std::cerr << std::endl;

		/* add all the directories recursively */
		syncDirectory(it->rootdir, nitem);
		it++;		
		i++;
		iTotal++;
	}

	rsiface->unlockData(); /* UnLock Interface */

	tree -> update();
}

void SharedFilesDialog::syncDirectory(const DirInfo &root, QTreeWidgetItem *parent)
{
	const std::list<DirInfo> &dirs = root.subdirs;
	const std::list<FileInfo> &files = root.files;

	std::list<DirInfo>::const_iterator it, eit;
	std::list<FileInfo>::const_iterator fit, efit;

	int i = 0;

	it  = dirs.begin();
	eit = dirs.end();

	fit  = files.begin();
	efit = files.end();

	int iTotal = parent->childCount();

	std::cerr << "SharedFilesDialog::syncDirectory() " << root.dirname;
	std::cerr << std::endl;
	std::cerr << "ItemChildCount: " << iTotal;
	std::cerr << std::endl;
	std::cerr << "syncData D:" << dirs.size() << " & F:" << files.size();
	std::cerr << std::endl;

	/* iterate through the directories */
	for(i = 0; it != eit;)
	{
		std::cerr << "syncDirEntry: " << it->dirname;
		std::cerr << std::endl;

		/* get both entries */
		QTreeWidgetItem *item = NULL;
		std::string dname;
		if (i < iTotal)
		{	
			item = parent-> child(i);
			dname = (item -> text(0)).toStdString();

			std::cerr << "syncDirEntry: DirItem: " << dname;
			std::cerr << std::endl;

			if ((item) && (item -> childCount() == 0))
			{
				std::cerr << "syncDirEntry: Not a Directory";
				std::cerr << std::endl;
				item = NULL; /* run out of directory entries */
			}
		}				

		/* ok case first */
		if ((item) && (dname == it->dirname))
		{
			std::cerr << "syncDirEntry: Okay";
			std::cerr << std::endl;

			/* increment both */
			syncDirectory(*it, item);
			it++;
			i++;
			continue;
		}

		/* remove item */
		if ((item) && (dname < it->dirname))
		{
			std::cerr << "syncDirEntry: Remove";
			std::cerr << std::endl;

           		QTreeWidgetItem *oitem = parent -> takeChild(i);
			if (oitem)
				delete oitem;
			iTotal--;
			continue;	
		}

		std::cerr << "syncDirEntry: Adding";
		std::cerr << std::endl;

		/* if we get here then we are adding it in */
           	QTreeWidgetItem *nitem = new QTreeWidgetItem();

		/* add all the labels */
		nitem -> setText(0, QString::fromStdString(it->dirname));
		std::ostringstream str1, str2;
		str1 << it->nofiles; /* No Entries */
		str2 << it->nobytes; /* Total Bytes */

		nitem -> setText(1, QString::fromStdString(str1.str()));
		nitem -> setText(2, QString::fromStdString(str2.str()));

		/* do we add ahead, after, or at the end? */
		if ((item) && (dname > it->dirname))
		{
			std::cerr << "syncDirEntry: Adding Ahead";
			std::cerr << std::endl;
			/* add ahead of item */
			parent -> insertChild (i, nitem);
		}
		else 
		{
			/* add to end */
			std::cerr << "syncDirEntry: Adding End";
			std::cerr << std::endl;
			parent -> insertChild (i, nitem);
		}
			
		std::cerr << "syncDirEntry: Sync SubDir";
		std::cerr << std::endl;

		/* add all the directories recursively */
		syncDirectory(*it, nitem);
		it++;		
		i++;
		iTotal++;
	}

	/* remove any directories left */
	bool dirsToRemove = true;
	while( dirsToRemove && ( i < iTotal ) )
	{
		/* get both entries */
		QTreeWidgetItem *oitem = parent-> child(i);
		if (oitem -> childCount() > 0)
		{
           		oitem = parent -> takeChild(i);
			if (oitem)
				delete oitem;
			iTotal--;
		}
		else
		{
			dirsToRemove = false;
		}
	}

	std::cerr << "syncFiles....";
	std::cerr << std::endl;

	/* do the same for the files */
	for(;fit != efit;)
	{
		/* get both entries */
		QTreeWidgetItem *item = NULL;
		std::string fname;
		if (i < iTotal)
		{	
			item = parent-> child(i);
			fname = (item -> text(0)).toStdString();
		}

		/* ok case first */
		if ((item) && (fname == fit->fname))
		{
			/* okay (or update data) */
			fit++;
			i++;
			continue;
		}

		/* remove item */
		if ((item) && (fname < fit->fname))
		{
           		QTreeWidgetItem *oitem = parent -> takeChild(i);
			if (oitem)
				delete oitem;
			iTotal--;
			continue;	
		}

		/* if we get here then we are adding it in */
           	QTreeWidgetItem *nitem = new QTreeWidgetItem();

		/* add all the labels */
		nitem -> setText(0, QString::fromStdString(fit->fname));
		std::ostringstream str1, str2;
		str1 << fit->size;  
		str2 << fit->rank;

		nitem -> setText(1, QString::fromStdString(str1.str()));
		nitem -> setText(2, QString::fromStdString(str2.str()));

		/* do we add ahead, after, or at the end? */
		if ((item) && (fname > fit->fname))
		{
			/* add ahead */
			parent -> insertChild (i, nitem);
		}
		else 
		{
			/* add to end */
			parent -> insertChild (i, nitem);
		}
			
		fit++;		
		i++;
		iTotal++;
	}

	/* remove any files left */
	while( i < iTotal )
	{
		/* get both entries */
		QTreeWidgetItem *oitem = parent-> takeChild(i);
		if (oitem)
			delete oitem;
		iTotal--;
	}

	if (!i)
	{
	  if (root.nofiles > 0)
	  {
		/* Add a <Empty> item */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		//item -> setText(0, QString::fromStdString(fit->fname));
		item -> setText(0, "<Loading ...>");
	  }
	  else
	  {
		/* Add a <Empty> item */
           	QTreeWidgetItem *item = new QTreeWidgetItem(parent);

		/* add all the labels */
		item -> setText(0, "<Empty>");
	  }
	}
		
	return;
}

#endif



void  SharedFilesDialog::preModDirectories(bool update_local)
{
	if (update_local)
	{
		localModel->preMods();
	}
	else
	{
		model->preMods();
	}
}


void  SharedFilesDialog::ModDirectories(bool update_local)
{
	if (update_local)
	{
		localModel->postMods();
	}
	else
	{
		model->postMods();
	}
}



