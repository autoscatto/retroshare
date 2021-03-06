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


#include "MessagesPopupDialog.h"
#include "msgs/ChanMsgDialog.h"
#include "util/printpreview.h"

#include "rsiface/rsiface.h"
#include "rsiface/rspeers.h"
#include "rsiface/rsmsgs.h"
#include "rsiface/rsfiles.h"
#include <sstream>

#include <QContextMenuEvent>
#include <QMenu>
#include <QCursor>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrinter>
#include <QDateTime>
#include <QHeaderView>


/* Images for context menu icons */
#define IMAGE_MESSAGE		   ":/images/folder-draft.png"
#define IMAGE_MESSAGEREPLY	   ":/images/mail_reply.png"
#define IMAGE_MESSAGEFORWARD	   ":/images/mail_forward.png"
#define IMAGE_MESSAGEREMOVE 	   ":/images/message-mail-imapdelete.png"
#define IMAGE_DOWNLOAD    	   ":/images/start.png"
#define IMAGE_DOWNLOADALL          ":/images/startall.png"


/** Constructor */
MessagesPopupDialog::MessagesPopupDialog(QWidget* parent, Qt::WFlags flags)
: QMainWindow(parent, flags)
{
  /* Invoke the Qt Designer generated object setup routine */
  ui.setupUi(this);

  connect( ui.messagesWidget, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( messageslistWidgetCostumPopupMenu( QPoint ) ) );
  connect( ui.messagesList, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( msgfilelistWidgetCostumPopupMenu( QPoint ) ) );
  connect( ui.messagesWidget, SIGNAL( itemClicked ( QTreeWidgetItem *, int) ), this, SLOT( updateMessages ( QTreeWidgetItem *, int) ) );
  connect( ui.messageslistWidget, SIGNAL( currentRowChanged ( int) ), this, SLOT( changeBox ( int) ) );
  
  /*connect(ui.newmessageButton, SIGNAL(clicked()), this, SLOT(newmessage()));
  connect(ui.removemessageButton, SIGNAL(clicked()), this, SLOT(removemessage()));
  connect(ui.replymessageButton, SIGNAL(clicked()), this, SLOT(replytomessage()));
  connect(ui.forwardmessageButton, SIGNAL(clicked()), this, SLOT(forwardmessage()));*/

  connect(ui.actionCompose, SIGNAL(triggered()), this, SLOT(newmessage()));
  connect(ui.actionReplyMessage, SIGNAL(triggered()), this, SLOT(removemessage()));
  connect(ui.actionForwardMessage, SIGNAL(triggered()), this, SLOT(replytomessage()));
  connect(ui.actionRemove, SIGNAL(triggered()), this, SLOT(forwardmessage()));

  //connect(ui.printbutton, SIGNAL(clicked()), this, SLOT(print()));
  connect(ui.actionPrint, SIGNAL(triggered()), this, SLOT(print()));
  connect(ui.actionPrintPreview, SIGNAL(triggered()), this, SLOT(printpreview()));

  connect(ui.expandFilesButton, SIGNAL(clicked()), this, SLOT(togglefileview()));
  connect(ui.downloadButton, SIGNAL(clicked()), this, SLOT(getallrecommended()));
  

  mCurrCertId = "";
  mCurrMsgId  = "";
  
  /* hide the Tree +/- */
  ui.messagesList->setRootIsDecorated( false );
  ui.messagesWidget->setRootIsDecorated( false );
  
  /* Set header resize modes and initial section sizes */
  QHeaderView * msgwheader = ui.messagesWidget->header () ;
  msgwheader->setResizeMode (0, QHeaderView::Custom);
  msgwheader->setResizeMode (3, QHeaderView::Interactive);
    
  msgwheader->resizeSection ( 0, 24 );
  msgwheader->resizeSection ( 2, 250 );
  msgwheader->resizeSection ( 3, 140 );
  
    /* Set header resize modes and initial section sizes */
	QHeaderView * msglheader = ui.messagesList->header () ;
   	msglheader->setResizeMode (0, QHeaderView::Interactive);
	msglheader->setResizeMode (1, QHeaderView::Interactive);
	msglheader->setResizeMode (2, QHeaderView::Interactive);
	msglheader->setResizeMode (3, QHeaderView::Interactive);
  
	msglheader->resizeSection ( 0, 200 );
	msglheader->resizeSection ( 1, 100 );
	msglheader->resizeSection ( 2, 100 );
	msglheader->resizeSection ( 3, 200 );
	
	/*ui.newmessageButton->setIcon(QIcon(QString(":/images/folder-draft24-pressed.png")));
    	ui.replymessageButton->setIcon(QIcon(QString(":/images/replymail-pressed.png")));
	ui.forwardmessageButton->setIcon(QIcon(QString(":/images/mailforward24-hover.png")));
    	ui.removemessageButton->setIcon(QIcon(QString(":/images/deletemail-pressed.png")));
    	ui.printbutton->setIcon(QIcon(QString(":/images/print24.png")));

	ui.forwardmessageButton->setToolTip(tr("Forward selected Message"));*/
 

    QMenu * printmenu2 = new QMenu();
    printmenu2->addAction(ui.actionPrint);
    printmenu2->addAction(ui.actionPrintPreview);
    ui.actionPrintMenu->setMenu(printmenu2);

	 ui.messagesWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	 ui.messagesWidget->sortItems( 3, Qt::DescendingOrder );


  /* Hide platform specific features */
#ifdef Q_WS_WIN

#endif
}

void MessagesPopupDialog::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Delete)
	{
		removemessage() ;
		e->accept() ;
	}
	else
		QMainWindow::keyPressEvent(e) ;
}

void MessagesPopupDialog::messageslistWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

      contextMnu.clear();

      newmsgAct = new QAction(QIcon(IMAGE_MESSAGE), tr( "New Message" ), this );
      connect( newmsgAct , SIGNAL( triggered() ), this, SLOT( newmessage() ) );
      
		int nn = ui.messagesWidget->selectedItems().size() ;

		if(nn > 1)
		{
			removemsgAct = new QAction(QIcon(IMAGE_MESSAGEREMOVE), tr( "Remove Messages" ), this );
			connect( removemsgAct , SIGNAL( triggered() ), this, SLOT( removemessage() ) );
			contextMnu.addAction( removemsgAct);
		}
		else if(nn == 1)
		{
			replytomsgAct = new QAction(QIcon(IMAGE_MESSAGEREPLY), tr( "Reply to Message" ), this );
			connect( replytomsgAct , SIGNAL( triggered() ), this, SLOT( replytomessage() ) );
			contextMnu.addAction( replytomsgAct);

			forwardmsgAct = new QAction(QIcon(IMAGE_MESSAGEFORWARD), tr( "Forward Message" ), this );
			connect( forwardmsgAct , SIGNAL( triggered() ), this, SLOT( forwardmessage() ) );
			contextMnu.addAction( forwardmsgAct);

			contextMnu.addSeparator(); 

			removemsgAct = new QAction(QIcon(IMAGE_MESSAGEREMOVE), tr( "Remove Message" ), this );
			connect( removemsgAct , SIGNAL( triggered() ), this, SLOT( removemessage() ) );
			contextMnu.addAction( removemsgAct);
		}

      contextMnu.addAction( newmsgAct);
      contextMnu.exec( mevent->globalPos() );
}


void MessagesPopupDialog::msgfilelistWidgetCostumPopupMenu( QPoint point )
{

      QMenu contextMnu( this );
      QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );

//      getRecAct = new QAction(QIcon(IMAGE_DOWNLOAD), tr( "Download" ), this );
//      connect( getRecAct , SIGNAL( triggered() ), this, SLOT( getcurrentrecommended() ) );
      
      getAllRecAct = new QAction(QIcon(IMAGE_DOWNLOADALL), tr( "Download All" ), this );
      connect( getAllRecAct , SIGNAL( triggered() ), this, SLOT( getallrecommended() ) );


      contextMnu.clear();
//      contextMnu.addAction( getRecAct);
      contextMnu.addAction( getAllRecAct);
      contextMnu.exec( mevent->globalPos() );
}

void MessagesPopupDialog::newmessage()
{
    ChanMsgDialog *nMsgDialog = new ChanMsgDialog(true);

    /* fill it in */
    //std::cerr << "MessagesDialog::newmessage()" << std::endl;
    nMsgDialog->newMsg();
    nMsgDialog->show();
    nMsgDialog->activateWindow();


    /* window will destroy itself! */
}

void MessagesPopupDialog::replytomessage()
{
	/* put msg on msgBoard, and switch to it. */

	std::string cid;
	std::string mid;

	if(!getCurrentMsg(cid, mid))
		return ;

	mCurrCertId = cid;
	mCurrMsgId  = mid;

	MessageInfo msgInfo;
	if (!rsMsgs -> getMessage(mid, msgInfo))
		return ;

	ChanMsgDialog *nMsgDialog = new ChanMsgDialog(true);
	/* fill it in */
	//std::cerr << "MessagesDialog::newmessage()" << std::endl;
	nMsgDialog->newMsg();
	nMsgDialog->insertTitleText( (QString("Re: ") + QString::fromStdWString(msgInfo.title)).toStdString()) ;
	nMsgDialog->setWindowTitle(tr("Re: ") + QString::fromStdWString(msgInfo.title) ) ;


	QTextDocument doc ;
	doc.setHtml(QString::fromStdWString(msgInfo.msg)) ;
	std::string cited_text(doc.toPlainText().toStdString()) ;

	nMsgDialog->insertPastedText(cited_text) ;
	nMsgDialog->addRecipient( msgInfo.srcId ) ;
	nMsgDialog->show();
	nMsgDialog->activateWindow();
}

void MessagesPopupDialog::forwardmessage()
{
	/* put msg on msgBoard, and switch to it. */

	std::string cid;
	std::string mid;

	if(!getCurrentMsg(cid, mid))
		return ;

	mCurrCertId = cid;
	mCurrMsgId  = mid;

	MessageInfo msgInfo;
	if (!rsMsgs -> getMessage(mid, msgInfo))
		return ;

	ChanMsgDialog *nMsgDialog = new ChanMsgDialog(true);
	/* fill it in */
	//std::cerr << "MessagesDialog::newmessage()" << std::endl;
	nMsgDialog->newMsg();
	nMsgDialog->insertTitleText( (QString("Fwd: ") + QString::fromStdWString(msgInfo.title)).toStdString()) ;
	nMsgDialog->setWindowTitle(tr("Fwd: ") + QString::fromStdWString(msgInfo.title) ) ;


	QTextDocument doc ;
	doc.setHtml(QString::fromStdWString(msgInfo.msg)) ;
	std::string cited_text(doc.toPlainText().toStdString()) ;

	nMsgDialog->insertForwardPastedText(cited_text) ;
	nMsgDialog->addRecipient( msgInfo.srcId ) ;
	nMsgDialog->show();
	nMsgDialog->activateWindow();
}

void MessagesPopupDialog::togglefileview()
{
	/* if msg header visible -> hide by changing splitter 
	 * three widgets...
	 */

	QList<int> sizeList = ui.msgSplitter->sizes();
	QList<int>::iterator it;

	int listSize = 0;
	int msgSize = 0;
	int recommendSize = 0;
	int i = 0;

	for(it = sizeList.begin(); it != sizeList.end(); it++, i++)
	{
		if (i == 0)
		{
			listSize = (*it);
		}
		else if (i == 1)
		{
			msgSize = (*it);
		}
		else if (i == 2)
		{
			recommendSize = (*it);
		}
	}

	int totalSize = listSize + msgSize + recommendSize;

	bool toShrink = true;
	if (recommendSize < (int) totalSize / 10)
	{
		toShrink = false;
	}

	QList<int> newSizeList;
	if (toShrink)
	{
		newSizeList.push_back(listSize + recommendSize / 3);
		newSizeList.push_back(msgSize + recommendSize * 2 / 3);
		newSizeList.push_back(0);
		ui.expandFilesButton->setIcon(QIcon(QString(":/images/edit_add24.png")));
	        ui.expandFilesButton->setToolTip("Expand");
	}
	else
	{
		/* no change */
		int nlistSize = (totalSize * 2 / 3) * listSize / (listSize + msgSize);
		int nMsgSize = (totalSize * 2 / 3) - listSize;
		newSizeList.push_back(nlistSize);
		newSizeList.push_back(nMsgSize);
		newSizeList.push_back(totalSize * 1 / 3);
	        ui.expandFilesButton->setIcon(QIcon(QString(":/images/edit_remove24.png")));
	        ui.expandFilesButton->setToolTip("Hide");
	}

	ui.msgSplitter->setSizes(newSizeList);
}


/* download the recommendations... */
void MessagesPopupDialog::getcurrentrecommended()
{
   

}

void MessagesPopupDialog::getallrecommended()
{
	/* get Message */
	MessageInfo msgInfo;
	if (!rsMsgs -> getMessage(mCurrMsgId, msgInfo))
	{
		return;
	}

        const std::list<FileInfo> &recList = msgInfo.files;
	std::list<FileInfo>::const_iterator it;

	std::list<std::string> fnames;
	std::list<std::string> hashes;
	std::list<int>         sizes;

	for(it = recList.begin(); it != recList.end(); it++)
	{
		fnames.push_back(it->fname);
		hashes.push_back(it->hash);
		sizes.push_back(it->size);
	}

	/* now do requests */
	std::list<std::string>::const_iterator fit;
	std::list<std::string>::const_iterator hit;
	std::list<int>::const_iterator sit;

	for(fit = fnames.begin(), hit = hashes.begin(), sit = sizes.begin(); 
		fit != fnames.end(); fit++, hit++, sit++)
	{
		std::cerr << "MessagesDialog::getallrecommended() Calling File Request";
		std::cerr << std::endl;
		std::list<std::string> srcIds;
		srcIds.push_back(msgInfo.srcId);
        	rsFiles -> FileRequest(*fit, *hit, *sit, "", 0, srcIds);
	}
}

void MessagesPopupDialog::changeBox( int newrow )
{
	//std::cerr << "MessagesDialog::changeBox()" << std::endl;
	insertMessages();
	insertMsgTxtAndFiles();
}

void MessagesPopupDialog::insertMessages()
{
	std::list<MsgInfoSummary> msgList;
	std::list<MsgInfoSummary>::const_iterator it;

	rsMsgs -> getMessageSummaries(msgList);

	/* get a link to the table */
        QTreeWidget *msgWidget = ui.messagesWidget;

	/* get the MsgId of the current one ... */

	std::string cid;
	std::string mid;

	bool oldSelected = getCurrentMsg(cid, mid);
	QTreeWidgetItem *newSelected = NULL;

	/* remove old items ??? */

	int listrow = ui.messageslistWidget -> currentRow();

	//std::cerr << "MessagesDialog::insertMessages()" << std::endl;
	//std::cerr << "Current Row: " << listrow << std::endl;

	/* check the mode we are in */
	unsigned int msgbox = 0;
	switch(listrow)
	{
		case 3:
			msgbox = RS_MSG_SENTBOX;
			break;
		case 2:
			msgbox = RS_MSG_DRAFTBOX;
			break;
		case 1:
			msgbox = RS_MSG_OUTBOX;
			break;
		case 0:
		default:
			msgbox = RS_MSG_INBOX;
			break;
	}

        QList<QTreeWidgetItem *> items;
	for(it = msgList.begin(); it != msgList.end(); it++)
	{
		/* check the message flags, to decide which
		 * group it should go in...
		 *
		 * InBox 
		 * OutBox 
		 * Drafts 
		 * Sent 
		 *
		 * FLAGS = OUTGOING.
		 * 	-> Outbox/Drafts/Sent
		 * 	  + SENT -> Sent
		 *	  + IN_PROGRESS -> Draft.
		 *	  + nuffing -> Outbox.
		 * FLAGS = INCOMING = (!OUTGOING)
		 * 	-> + NEW -> Bold.
		 *
		 */

		if ((it -> msgflags & RS_MSG_BOXMASK) != msgbox)
		{
			//std::cerr << "Msg from other box: " << it->msgflags;
			//std::cerr << std::endl;
			continue;
		}
		/* make a widget per friend */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);

		/* So Text should be:
		 * (1) Msg / Broadcast
		 * (1b) Person / Channel Name
		 * (2) Rank
		 * (3) Date
		 * (4) Title
		 * (5) Msg
		 * (6) File Count
		 * (7) File Total
		 */

		// Date First.... (for sorting)
		{
			QDateTime qtime;
			qtime.setTime_t(it->ts);
			QString timestamp = qtime.toString("yyyy-MM-dd hh:mm:ss");
			item -> setText(3, timestamp);
		}

		//  From ....
		{
			item -> setText(1, QString::fromStdString(rsPeers->getPeerName(it->srcId)));
		}

		// Subject
		item -> setText(2, QString::fromStdWString(it->title));
		item -> setIcon(2, (QIcon(":/images/message-mail-read.png")));

		// No of Files.
		{
			std::ostringstream out;
			out << it -> count;
			item -> setText(0, QString::fromStdString(out.str()));
			item -> setTextAlignment( 0, Qt::AlignCenter );
		}

		item -> setText(4, QString::fromStdString(it->srcId));
		item -> setText(5, QString::fromStdString(it->msgId));
		if ((oldSelected) && (mid == it->msgId))
		{
			newSelected = item;
		}

		if (it -> msgflags & RS_MSG_NEW)
		{
			for(int i = 0; i < 10; i++)
			{
				QFont qf = item->font(i);
				qf.setBold(true);
				item->setFont(i, qf);
			    item -> setIcon(2, (QIcon(":/images/message-mail.png")));

				//std::cerr << "Setting Item BOLD!" << std::endl;
			}
		}

		/* add to the list */
		items.append(item);
	}

	/* add the items in! */
	msgWidget->clear();
	msgWidget->insertTopLevelItems(0, items);

	if (newSelected)
	{
		msgWidget->setCurrentItem(newSelected);
	}
}

void MessagesPopupDialog::updateMessages( QTreeWidgetItem * item, int column )
{
	//std::cerr << "MessagesDialog::insertMsgTxtAndFiles()" << std::endl;
	insertMsgTxtAndFiles();
}


void MessagesPopupDialog::insertMsgTxtAndFiles()
{
	/* Locate the current Message */
	QTreeWidget *msglist = ui.messagesWidget;

	std::cerr << "MessagesDialog::insertMsgTxtAndFiles()" << std::endl;


	/* get its Ids */
	std::string cid;
	std::string mid;

	QTreeWidgetItem *qtwi = msglist -> currentItem();
	if (!qtwi)
	{
		/* blank it */
		ui.dateText-> setText("");
		ui.toText->setText("");
		ui.fromText->setText("");
		ui.filesText->setText("");

		ui.subjectText->setText("");
		ui.messagesList->clear();

		return;
	}
	else
	{
		cid = qtwi -> text(4).toStdString();
		mid = qtwi -> text(5).toStdString(); 
	}

	std::cerr << "MessagesDialog::insertMsgTxtAndFiles() mid:" << mid << std::endl;

	/* Save the Data.... for later */

	mCurrCertId = cid;
	mCurrMsgId  = mid;

	MessageInfo msgInfo;
	if (!rsMsgs -> getMessage(mid, msgInfo))
	{
		std::cerr << "MessagesDialog::insertMsgTxtAndFiles() Couldn't find Msg" << std::endl;
		return;
	}

        const std::list<FileInfo> &recList = msgInfo.files;
	std::list<FileInfo>::const_iterator it;

	/* get a link to the table */
	QTreeWidget *tree = ui.messagesList;

	/* get the MessageInfo */

        tree->clear(); 
        tree->setColumnCount(5); 

        QList<QTreeWidgetItem *> items;
	for(it = recList.begin(); it != recList.end(); it++)
	{
		/* make a widget per person */
           	QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0);
		/* (0) Filename */
		item -> setText(0, QString::fromStdString(it->fname));
		//std::cerr << "Msg FileItem(" << it->fname.length() << ") :" << it->fname << std::endl;
			
		/* (1) Size */
		{
			std::ostringstream out;
			out << it->size;
			item -> setText(1, QString::fromStdString(out.str()));
		}
		/* (2) Rank */
		{
			std::ostringstream out;
			out << it->rank;
			item -> setText(2, QString::fromStdString(out.str()));
		}
			
		item -> setText(3, QString::fromStdString(it->hash));
			
		/* add to the list */
		items.append(item);
	}

	/* add the items in! */
	tree->insertTopLevelItems(0, items);

	/* iterate through the sources */
	std::list<std::string>::const_iterator pit;

	QString msgTxt;
	for(pit = msgInfo.msgto.begin(); pit != msgInfo.msgto.end(); pit++)
	{
		msgTxt += QString::fromStdString(*pit);
		msgTxt += " <";
		msgTxt += QString::fromStdString(rsPeers->getPeerName(*pit));
		msgTxt += ">, ";
	}

	if (msgInfo.msgcc.size() > 0)
		msgTxt += "\nCc: ";
	for(pit = msgInfo.msgcc.begin(); pit != msgInfo.msgcc.end(); pit++)
	{
		msgTxt += QString::fromStdString(*pit);
		msgTxt += " <";
		msgTxt += QString::fromStdString(rsPeers->getPeerName(*pit));
		msgTxt += ">, ";
	}

	if (msgInfo.msgbcc.size() > 0)
		msgTxt += "\nBcc: ";
	for(pit = msgInfo.msgbcc.begin(); pit != msgInfo.msgbcc.end(); pit++)
	{
		msgTxt += QString::fromStdString(*pit);
		msgTxt += " <";
		msgTxt += QString::fromStdString(rsPeers->getPeerName(*pit));
		msgTxt += ">, ";
	}

	{
		QDateTime qtime;
		qtime.setTime_t(msgInfo.ts);
		QString timestamp = qtime.toString("yyyy-MM-dd hh:mm:ss");
		ui.dateText-> setText(timestamp);
	}
	ui.toText->setText(msgTxt);
	ui.fromText->setText(QString::fromStdString(rsPeers->getPeerName(msgInfo.srcId)));

	ui.subjectText->setText(QString::fromStdWString(msgInfo.title));
	ui.messagesText->setHtml(QString::fromStdWString(msgInfo.msg));

	{
		std::ostringstream out;
		out << "(" << msgInfo.count << " Files)";
		ui.filesText->setText(QString::fromStdString(out.str()));
	}

	std::cerr << "MessagesDialog::insertMsgTxtAndFiles() Msg Displayed OK!" << std::endl;

	/* finally mark message as read! */
	rsMsgs -> MessageRead(mid);
}


bool MessagesPopupDialog::getCurrentMsg(std::string &cid, std::string &mid)
{
	/* Locate the current Message */
	QTreeWidget *msglist = ui.messagesWidget;

	//std::cerr << "MessagesDialog::getCurrentMsg()" << std::endl;

	/* get its Ids */
	QTreeWidgetItem *qtwi = msglist -> currentItem();
	if (qtwi)
	{
		cid = qtwi -> text(4).toStdString();
		mid = qtwi -> text(5).toStdString();
		return true;
	}
	return false;
}


void MessagesPopupDialog::removemessage()
{
#ifdef TO_REMOVE
	//std::cerr << "MessagesDialog::removemessage()" << std::endl;
	std::string cid, mid;
	if (!getCurrentMsg(cid, mid))
	{
		//std::cerr << "MessagesDialog::removemessage()";
		//std::cerr << " No Message selected" << std::endl;
		return;
	}
#endif

	QList<QTreeWidgetItem*> list(ui.messagesWidget->selectedItems()) ; 

	for(QList<QTreeWidgetItem*>::const_iterator it(list.begin());it!=list.end();++it)
		rsMsgs->MessageDelete((*it)->text(5).toStdString());

	return;
}


void MessagesPopupDialog::markMsgAsRead()
{
	//std::cerr << "MessagesDialog::markMsgAsRead()" << std::endl;
	std::string cid, mid;
	if (!getCurrentMsg(cid, mid))
	{
		//std::cerr << "MessagesDialog::markMsgAsRead()";
		//std::cerr << " No Message selected" << std::endl;
		return;
	}

	rsMsgs -> MessageRead(mid);

	return;
}

void MessagesPopupDialog::print()
{
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (ui.messagesText->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        ui.messagesText->print(&printer);
    }
    delete dlg;
#endif
}

void MessagesPopupDialog::printpreview()
{
    PrintPreview *preview = new PrintPreview(ui.messagesText->document(), this);
    preview->setWindowModality(Qt::WindowModal);
    preview->setAttribute(Qt::WA_DeleteOnClose);
    preview->show();
}

