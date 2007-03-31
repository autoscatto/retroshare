
#include "rsiface/notifyqt.h"

#include "gui/ConnectionsDialog.h"
#include "gui/PeersDialog.h"
#include "gui/SharedFilesDialog.h"

#include <iostream>
#include <sstream>

void NotifyQt::notifyErrorMsg(int list, int type, std::string msg)
{
	return;
}

void NotifyQt::notifyChat()
{
	return;
}

void NotifyQt::notifyListChange(int list, int type)
{
	std::cerr << "NotifyQt::notifyListChange()" << std::endl;
	switch(list)
	{
		case NOTIFY_LIST_NEIGHBOURS:
			displayNeighbours();
			break;
		case NOTIFY_LIST_FRIENDS:
			displayFriends();
			break;
		case NOTIFY_LIST_DIRLIST:
			displayDirectories();
			break;
		case NOTIFY_LIST_SEARCHLIST:
			displaySearch();
			break;
		case NOTIFY_LIST_MESSAGELIST:
			displayMessages();
			break;
		case NOTIFY_LIST_CHANNELLIST:
			displayChannels();
			break;
		case NOTIFY_LIST_TRANSFERLIST:
			displayTransfers();
			break;
		default:
			break;
	}
	return;
}


void NotifyQt::notifyListPreChange(int list, int type)
{
	std::cerr << "NotifyQt::notifyListPreChange()" << std::endl;
	switch(list)
	{
		case NOTIFY_LIST_NEIGHBOURS:
			preDisplayNeighbours();
			break;
		case NOTIFY_LIST_FRIENDS:
			preDisplayFriends();
			break;
		case NOTIFY_LIST_DIRLIST:
			preDisplayDirectories();
			break;
		case NOTIFY_LIST_SEARCHLIST:
			preDisplaySearch();
			break;
		case NOTIFY_LIST_MESSAGELIST:
			preDisplayMessages();
			break;
		case NOTIFY_LIST_CHANNELLIST:
			preDisplayChannels();
			break;
		case NOTIFY_LIST_TRANSFERLIST:
			preDisplayTransfers();
			break;
		default:
			break;
	}
	return;
}

			
			
void NotifyQt::displayNeighbours()
{
	iface->lockData(); /* Lock Interface */

	std::map<RsCertId,NeighbourInfo>::const_iterator it;
	const std::map<RsCertId,NeighbourInfo> &neighs = iface->getNeighbourMap();

	std::ostringstream out;
        for(it = neighs.begin(); it != neighs.end(); it++)
        {
		out << "Neighbour: ";
		out << it ->second.name << " ";
		out << it ->second.status << " ";
		out << it ->second.trustLvl << " ";
		out << std::endl;
	}
	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */

	/* Do the GUI */
	cDialog->insertConnect();
}

void NotifyQt::displayFriends()
{
	iface->lockData(); /* Lock Interface */

	std::map<RsCertId,NeighbourInfo>::const_iterator it;
	const std::map<RsCertId,NeighbourInfo> &friends = iface->getFriendMap();

	std::ostringstream out;
        for(it = friends.begin(); it != friends.end(); it++)
        {
		out << "Friend: ";
		out << it->second.name << " ";
		out << it->second.status << " ";
		out << it->second.trustLvl << " ";
		out << std::endl;
	}
	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */

	pDialog->insertPeers();
}

void NotifyQt::preDisplayDirectories()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	out << "NotifyQt::preDisplayDirectories()" << std::endl;

	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */

	dDialog->preModDirectories(false);  /* Remote */
}


void NotifyQt::displayDirectories()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	out << "NotifyQt::displayDirectories()" << std::endl;

	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */

	//dDialog->insertFiles(true);  /* Local  */
	//dDialog->insertFiles(false); /* Remote */
	//dDialog->syncDirectories(true);  /* Local  */
	//dDialog->syncDirectories(false); /* Remote */

	dDialog->ModDirectories(false);  /* Remote */
}


void NotifyQt::displaySearch()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */
}


void NotifyQt::displayMessages()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	std::cerr << out.str();

	std::list<MessageInfo>::const_iterator it;
	const std::list<MessageInfo> &msgs = iface->getMessages();

	std::list<FileInfo>::const_iterator fit;
	int i;

	for(it = msgs.begin(); it != msgs.end(); it++)
	{
		out << "Message: ";
		out << it->title << std::endl;
 		out << "\t" << it->msg << std::endl;
		const std::list<FileInfo> &files = it -> files;
		for(fit = files.begin(), i = 1; fit != files.end(); fit++, i++)
		{
 			out << "\t\tFile(" << i << ") " << fit->fname << std::endl;
		}
		out << std::endl;
	}

	iface->unlockData(); /* UnLock Interface */
}

void NotifyQt::displayChannels()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */
}


void NotifyQt::displayTransfers()
{
	iface->lockData(); /* Lock Interface */

	std::ostringstream out;
	std::cerr << out.str();

	iface->unlockData(); /* UnLock Interface */
}


void NotifyQt::preDisplayNeighbours()
{

}

void NotifyQt::preDisplayFriends()
{

}

void NotifyQt::preDisplaySearch()
{

}

void NotifyQt::preDisplayMessages()
{

}

void NotifyQt::preDisplayChannels()
{

}

void NotifyQt::preDisplayTransfers()
{


}



