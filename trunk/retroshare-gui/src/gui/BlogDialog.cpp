/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2008 Robert Fernie
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

#include "BlogDialog.h"

#include "rsiface/rsQblog.h"
#include "rsiface/rspeers.h" //to retrieve peer/usrId info

#include "profile/ProfileView.h"

/** Constructor */
BlogDialog::BlogDialog(QWidget *parent)
: MainPage (parent)
{
  /* Invoke the Qt Designer generated object setup routine */
  setupUi(this);
  
  connect(sendBtn, SIGNAL(clicked()), this, SLOT(sendBlog()));
  //connect(statusBtn, SIGNAL(clicked()), this, SLOT(setStatus()));
  connect(boldBtn, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(underlineBtn, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(italicBtn, SIGNAL(clicked()), this, SLOT(setFont()));
  connect(refreshBtn, SIGNAL(clicked()), this, SLOT(update()));
 
  connect(userList, SIGNAL( customContextMenuRequested( QPoint ) ), this, SLOT( peerCustomPopupMenu( QPoint ) ) );
    
  /* Current Font */
  mCurrentFont = QFont("Comic Sans MS", 8);

  /* Font for username and timestamp */
  mUsrFont = QFont("Comic Sans MS", 8);
  
}


void BlogDialog::peerCustomPopupMenu( QPoint point )
{
	/* 
	 */
	
	QMenu contextMnu( this );
	QMouseEvent *mevent = new QMouseEvent( QEvent::MouseButtonPress, point, Qt::RightButton, Qt::RightButton, Qt::NoModifier );
	
	QAction *profileAct = new QAction( tr( "Show Profile" ), this );
	QAction *onlyoneAct = new QAction( tr( "Show Only This Blog" ), this );
	QAction *allAct = new QAction( tr( "Show All Blogs" ), this );
	
	connect( profileAct , SIGNAL( triggered() ), this, SLOT( showuserprofile() ) );
	connect( onlyoneAct , SIGNAL( triggered() ), this, SLOT( showoneblog() ) );
	connect( allAct , SIGNAL( triggered() ), this, SLOT( showallblogs() ) );
	
	contextMnu.clear();
	contextMnu.addAction( profileAct );
	contextMnu.addAction( onlyoneAct );
	contextMnu.addAction( allAct );
	contextMnu.exec( mevent->globalPos() );

}

void BlogDialog::showuserprofile()
{
	showprofile("");
}

void BlogDialog::showprofile(std::string id)
{
static ProfileView *oneProfile = new ProfileView(NULL);

	oneProfile -> setPeerId(id);
	oneProfile -> show();
}

void BlogDialog::showoneblog()
{
	return;
}


void BlogDialog::showallblogs()
{
	return;
}


void BlogDialog::sendBlog()
{
	QString blogMsg = lineEdit->toPlainText();

	 if(blogMsg == "")
	 {
	 	QMessageBox::information(this, tr("No message entered"),
        tr("Please enter a message."),QMessageBox::Ok,
        QMessageBox::Ok);
	 }
	 else
	 {
	 	/* note: timing will be handled by core */
		std::string blog = blogMsg.toStdString();
		rsQblog->sendBlog(blog); 
	 }	
	  
	lineEdit->clear();//Clear lineEdit
	lineEdit->setFocus(); //setFocus on lineEdit
}

void BlogDialog::setFont()
{
	mCurrentFont.setUnderline(underlineBtn->isChecked());
	mCurrentFont.setItalic(italicBtn->isChecked());
	mCurrentFont.setBold(boldBtn->isChecked());
	lineEdit->setFont(mCurrentFont);
	lineEdit->setFocus();
}

/*
 * this will also send to core, to blog "updated"
 */
void BlogDialog::setStatus()
{
	QString statusMsg = lineEdit->toPlainText();
	
	std::list<std::string> UsrList;
	
	/* test to see if load dummy data worked ! */

	 	if(statusMsg == "")
	 	{
	 		QMessageBox::information(this, tr("No message"),
            tr("Please enter a message."),QMessageBox::Ok,
            QMessageBox::Ok);
	 	}
	 	else
	 	{
	 		rsQblog->setStatus("whatsup");
	 	}
	 
	/* Clear lineEdit */
	lineEdit->clear();
	
	/* setFocus on lineEdit **/
	lineEdit->setFocus();
		
}

void BlogDialog::addUser(const std::string &usr)
{
	QTreeWidgetItem *NewUser = new QTreeWidgetItem(userList);
	NewUser->setText(0, tr(usr.c_str()));	
	//TODO can add status as subtree to each user (use rsQblog.getStatus(usr) )	
}

void BlogDialog::clear(void)
{
	blogText->clear();
}
	
void BlogDialog::update(void)
{
	updateUserList();
	updateBlogs();
}

void BlogDialog::updateBlogs(void)
{
	rsQblog->getFilterSwitch();
	
	std::map<std::string, std::string> UsrStatus;
	
	if(!rsQblog->getStatus(UsrStatus))
		std::cerr << "failed to get usr status" << std::endl;
	
	clear(); //create a clear screen
	 
	/* retrieve usr names and populate usr list bar */
	
	std::list<std::string> usrList; 
	QString TempVar; // to convert numerics to string note: tbd find way to avoid temporary
	
	if (!rsPeers)
	{
		/* not ready yet! */
		return;
	}
	
	if(!rsPeers->getFriendList(usrList))
		std::cerr << "failed to get friend list";
	
	
	usrList.push_back(rsPeers->getOwnId()); // add your id
	
	/* populate the blog msgs screen */
	
	std::map< std::string, std::multimap<long int, std::string> > blogs; // to store blogs
	
	if(!rsQblog->getBlogs(blogs))
		std::cerr << "failed to get blogs" << std::endl;
	
	/* print usr name and their blogs to screen */
	for(std::list<std::string>::iterator it = usrList.begin(); it !=usrList.end(); it++)
	{	


		TempVar = rsPeers->getPeerName(*it).c_str(); // store usr name in temporary
		blogText->setTextColor(QColor(255, 0, 0, 255));
		blogText->setCurrentFont(mUsrFont); // make bold for username		
		blogText->append("\n" + TempVar); // write usr name to screen
		std::cerr << "creating usr tree" << std::endl;
		
		/*print blog time-posted/msgs to screen*/
		
		std::multimap<long int, std::string>::reverse_iterator blogIt =  blogs[*it].rbegin(); 
		
		if(blogs[*it].empty())
		{
			std::cerr << "usr blog empty!"  << std::endl;
			continue; 	
		}
	
		for( ; blogIt != blogs[*it].rend(); blogIt++)
		{	
			std::cerr << "now printing blogs" << std::endl;
			time_t postedTime = blogIt->first;
			time(&postedTime); //convert to human readable time
			blogText->setTextColor(QColor(255, 0, 0, 255)); // 
			blogText->setCurrentFont(mUsrFont); // make bold for posted date
			blogText->append("\nPosted: " + QString (ctime(&postedTime))); // print time of blog to screen 
			blogText->setCurrentFont(mCurrentFont); // reset the font for blog messages
			blogText->setTextColor(QColor(0, 0, 0, 255)); // set back color to black
			blogText->append(blogIt->second.c_str()); // print blog msg to screen 
		}	
			
	}	
}
	
	
void BlogDialog::updateUserList(void)
{
	/* retrieve usr names and populate usr list bar */
	
	std::list<std::string> usrList; 
	/* get existing list ... */
	std::list<std::string> filterList;

	std::list<std::string>::iterator it;
	if ((!rsPeers) || (!rsQblog))
	{
		/* not ready yet! */
		return;
	}
	
	rsPeers->getFriendList(usrList);
	usrList.push_back(rsPeers->getOwnId()); // add your id

	//rsQblog->getFilterList(filterList);
	
	
	userList->clear();

	/* print usr name and their blogs to screen */
	for(it = usrList.begin(); it !=usrList.end(); it++)
	{	
		QTreeWidgetItem *item = new QTreeWidgetItem(userList);

		bool active = false;
		if (filterList.end() != std::find(filterList.begin(), filterList.end(), *it))
			active = true;

		item->setText(0, QString::fromStdString(rsPeers->getPeerName(*it))); // add usr to Qwidget tree
		item->setText(1, QString::fromStdString(*it)); // add usr id.

                if (active)
		{
			item -> setCheckState(0, Qt::Checked);
		}
		else
		{
			item -> setCheckState(0, Qt::Unchecked);
		}
	}
}
	
	


