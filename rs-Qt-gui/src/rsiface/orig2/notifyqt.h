#ifndef RSIFACE_NOTIFY_TXT_H
#define RSIFACE_NOTIFY_TXT_H

#include "rsiface/rsiface.h"

#include <string>

class ConnectionsDialog;
class PeersDialog;
class SharedFilesDialog;

class NotifyQt: public NotifyBase
{
        public:
        NotifyQt() : cDialog(NULL), pDialog(NULL)
	{ return; }

        virtual ~NotifyQt() { return; }

	void setConnectionDialog(ConnectionsDialog *c) { cDialog = c; }
	void setPeersDialog(PeersDialog *p) { pDialog = p; }
	void setDirDialog(SharedFilesDialog *d) { dDialog = d; }

	void setRsIface(RsIface *i) { iface = i; }

virtual void notifyListPreChange(int list, int type);
virtual void notifyListChange(int list, int type);
virtual void notifyErrorMsg(int list, int sev, std::string msg);
virtual void notifyChat();

	private:

	void displayNeighbours();
	void displayFriends();
	void displayDirectories();
	void displaySearch();
	void displayMessages();
	void displayChannels();
	void displayTransfers();

	void preDisplayNeighbours();
	void preDisplayFriends();
	void preDisplayDirectories();
	void preDisplaySearch();
	void preDisplayMessages();
	void preDisplayChannels();
	void preDisplayTransfers();

	/* so we can update windows */
	ConnectionsDialog *cDialog;
	PeersDialog       *pDialog;
	SharedFilesDialog *dDialog;
	RsIface           *iface;
};

#endif
