
#include "rsiface/notifyqt.h"
#include "rsiface/rsnotify.h"
#include "rsiface/rspeers.h"
#include "rsiface/rsphoto.h"
#ifdef TURTLE_HOPPING
#include <rsiface/rsturtle.h>
#endif

#include "gui/NetworkDialog.h"
#include "gui/PeersDialog.h"
#include "gui/SharedFilesDialog.h"
#include "gui/TransfersDialog.h"
//#include "gui/ChatDialog.h"
#include "gui/MessagesDialog.h"
#include "gui/ChannelsDialog.h"
#include "gui/MessengerWindow.h"

#include "gui/toaster/OnlineToaster.h"
#include "gui/toaster/MessageToaster.h"
#include "gui/toaster/ChatToaster.h"
#include "gui/toaster/CallToaster.h"

#include "gui/Preferences/rsharesettings.h"

#include <iostream>
#include <sstream>

/*****
 * #define NOTIFY_DEBUG
 ****/

void NotifyQt::notifyErrorMsg(int list, int type, std::string msg)
{
	(void) list;
	(void) type;
	(void) msg;

	return;
}

void NotifyQt::notifyChatStatus(const std::string& peer_id,const std::string& status_string)
{
	std::cerr << "Received chat status string: " << status_string << std::endl ;
	emit chatStatusChanged(QString::fromStdString(peer_id),QString::fromStdString(status_string)) ;
}

#ifdef TURTLE_HOPPING
void NotifyQt::notifyTurtleSearchResult(uint32_t search_id,const std::list<TurtleFileInfo>& files) 
{
	std::cerr << "in notify search result..." << std::endl ;

	for(std::list<TurtleFileInfo>::const_iterator it(files.begin());it!=files.end();++it)
		emit gotTurtleSearchResult(search_id,*it) ;
}
#endif
void NotifyQt::notifyHashingInfo(std::string fileinfo)
{
	emit hashingInfoChanged(QString::fromStdString(fileinfo)) ;
}

//void NotifyQt::notifyChat()
//{
//	std::cerr << "Received chat notification" << std::endl ;
//	return;
//}

void NotifyQt::notifyListChange(int list, int type)
{
	(void) type;

#ifdef NOTIFY_DEBUG
	std::cerr << "NotifyQt::notifyListChange()" << std::endl;
#endif
	switch(list)
	{
		case NOTIFY_LIST_NEIGHBOURS:
#ifdef DEBUG
			std::cerr << "received neighbrs changed" << std::endl ;
#endif
			emit neighborsChanged();
			break;
		case NOTIFY_LIST_FRIENDS:
#ifdef DEBUG
			std::cerr << "received friends changed" << std::endl ;
#endif
			emit friendsChanged() ;
			break;
		case NOTIFY_LIST_DIRLIST:
#ifdef DEBUG
			std::cerr << "received files changed" << std::endl ;
#endif
			emit filesPostModChanged(false) ;	/* Remote */
			emit filesPostModChanged(true) ;  /* Local */
			break;
		case NOTIFY_LIST_SEARCHLIST:
			break;
		case NOTIFY_LIST_MESSAGELIST:
#ifdef DEBUG
			std::cerr << "received msg changed" << std::endl ;
#endif
			emit messagesChanged() ;
			break;
		case NOTIFY_LIST_CHANNELLIST:
			break;
		case NOTIFY_LIST_TRANSFERLIST:
#ifdef DEBUG
			std::cerr << "received transfer changed" << std::endl ;
#endif
			emit transfersChanged() ;
			break;
		case NOTIFY_LIST_CONFIG:
#ifdef DEBUG
			std::cerr << "received config changed" << std::endl ;
#endif
			emit configChanged() ;
			break ;
		default:
			break;
	}
	return;
}


void NotifyQt::notifyListPreChange(int list, int type)
{
#ifdef NOTIFY_DEBUG
	std::cerr << "NotifyQt::notifyListPreChange()" << std::endl;
#endif
	switch(list)
	{
		case NOTIFY_LIST_NEIGHBOURS:
			break;
		case NOTIFY_LIST_FRIENDS:
			emit friendsChanged() ;
			break;
		case NOTIFY_LIST_DIRLIST:
			emit filesPreModChanged(false) ;	/* remote */
			emit filesPreModChanged(true) ;	/* local */
			break;
		case NOTIFY_LIST_SEARCHLIST:
			break;
		case NOTIFY_LIST_MESSAGELIST:
			break;
		case NOTIFY_LIST_CHANNELLIST:
			break;
		case NOTIFY_LIST_TRANSFERLIST:
			break;
		default:
			break;
	}
	return;
}

	/* New Timer Based Update scheme ...
	 * means correct thread seperation
	 *
	 * uses Flags, to detect changes 
	 */

void NotifyQt::UpdateGUI()
{
	/* hack to force updates until we've fixed that part */
	static  time_t lastTs = 0;

//	std::cerr << "Got update signal t=" << lastTs << std::endl ;

	lastTs = time(NULL) ;

	static bool already_updated = false ;	// these only update once at start because they may already have been set before 
														// the gui is running, then they get updated by callbacks.
	if(!already_updated)
	{
		emit messagesChanged() ;
		emit neighborsChanged();
		emit configChanged();

		already_updated = true ;
	}
	
	/* Finally Check for PopupMessages / System Error Messages */

	if (rsNotify)
	{
		uint32_t sysid;
		uint32_t type;
		std::string title, id, msg;
		
		if (rsNotify->NotifyPopupMessage(type, id, msg))
		{
			RshareSettings settings;
			uint popupflags = settings.getNotifyFlags();

			/* id the name */
			std::string name = rsPeers->getPeerName(id);
			std::string realmsg = msg + "<strong>" + name + "</strong>";
			switch(type)
			{
				case RS_POPUP_MSG:
				if (popupflags & RS_POPUP_MSG)
				{
					MessageToaster * msgToaster = new MessageToaster();
					msgToaster->setMessage(QString::fromStdString(realmsg));
					msgToaster->show();
				}
					break;
				case RS_POPUP_CHAT:
				if (popupflags & RS_POPUP_CHAT)
				{
					ChatToaster * chatToaster = new ChatToaster();
					chatToaster->setMessage(QString::fromStdString(realmsg));
					chatToaster->show();
				}
					break;
				case RS_POPUP_CALL:
				if (popupflags & RS_POPUP_CALL)
				{
					CallToaster * callToaster = new CallToaster();
					callToaster->setMessage(QString::fromStdString(realmsg));
					callToaster->show();
				}
					break;
				default:
				case RS_POPUP_CONNECT:
				if (popupflags & RS_POPUP_CONNECT)
				{
					OnlineToaster * onlineToaster = new OnlineToaster();
					onlineToaster->setMessage(QString::fromStdString(realmsg));
					onlineToaster->show();
				}
					break;
			}

		}

		if (rsNotify->NotifySysMessage(sysid, type, title, msg))
		{
			/* make a warning message */
			switch(type)
			{
				case RS_SYS_ERROR:
					QMessageBox::critical(0, 
							QString::fromStdString(title), 
							QString::fromStdString(msg));
					break;
				case RS_SYS_WARNING:
					QMessageBox::warning(0, 
							QString::fromStdString(title), 
							QString::fromStdString(msg));
					break;
				default:
				case RS_SYS_INFO:
					QMessageBox::information(0, 
							QString::fromStdString(title), 
							QString::fromStdString(msg));
					break;
			}
		}
		if (rsNotify->NotifyLogMessage(sysid, type, title, msg))
		{
			/* make a log message */
			std::string logMesString = title + " " + msg;
			switch(type)
			{
				case RS_SYS_ERROR:
				case RS_SYS_WARNING:
				case RS_SYS_INFO:		 emit logInfoChanged(QString(logMesString.c_str()));
				default:;
			}
		}
	}
}
			
//void NotifyQt::displaySearch()
//{
//	iface->lockData(); /* Lock Interface */
//
//#ifdef NOTIFY_DEBUG
//	std::ostringstream out;
//	std::cerr << out.str();
//#endif
//
//	iface->unlockData(); /* UnLock Interface */
//}

//  void NotifyQt::displayChat()
//  {
//  	iface->lockData(); /* Lock Interface */
//  
//  #ifdef NOTIFY_DEBUG
//  	std::ostringstream out;
//  	std::cerr << out.str();
//  #endif
//  
//  	iface->unlockData(); /* UnLock Interface */
//  
//  	if (hDialog)
//   		hDialog -> insertChat();
//  }
//
//
//void NotifyQt::displayChannels()
//{
//	iface->lockData(); /* Lock Interface */
//
//#ifdef NOTIFY_DEBUG
//	std::ostringstream out;
//	std::cerr << out.str();
//#endif
//
//	iface->unlockData(); /* UnLock Interface */
//
//	if (sDialog)
// 		sDialog -> insertChannels();
//}
//
//
//void NotifyQt::displayTransfers()
//{
//	/* Do the GUI */
//	if (tDialog)
//		tDialog->insertTransfers();
//}
//
//
//void NotifyQt::preDisplayNeighbours()
//{
//
//}
//
//void NotifyQt::preDisplayFriends()
//{
//
//}

//void NotifyQt::preDisplaySearch()
//{
//
//}
//
//void NotifyQt::preDisplayMessages()
//{
//
//}
//
//void NotifyQt::preDisplayChannels()
//{
//
//}
//
//void NotifyQt::preDisplayTransfers()
//{
//
//
//}



