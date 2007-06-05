
#include "dht/dhthandler.h"


/* This stuff is actually C */

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef  __cplusplus
} /* extern C */
#endif
/* This stuff is actually C */




/* HACK TO SWITCH THIS OFF during testing */
/*define  NO_UPNP_RUNNING  1*/

#include "upnp/upnputil.h"
#include "upnp/upnphandler.h"

class uPnPConfigData
{
	public:
		struct UPNPDev * devlist;
		struct UPNPUrls urls;
		struct IGDdatas data;
	        char lanaddr[16];	/* my ip address on the LAN */
};

#include <iostream>
#include <sstream>


void upnphandler::run()
{

	/* infinite loop */
	while(1)
	{
		std::cerr << "UPnPHandler::Run()" << std::endl;
		int allowedSleep = 30; /* check every 30 seconds */

		/* lock it up */
		dataMtx.lock(); /* LOCK MUTEX */

		bool shutdown = toShutdown;
		int state = upnpState;

		dataMtx.unlock(); /* UNLOCK MUTEX */

		if (shutdown)
		{
			return;
		}

		/* do the work! */
		checkUPnPState();

		/* check new state for sleep period */

		dataMtx.lock(); /* LOCK MUTEX */

		state = upnpState;

		dataMtx.unlock(); /* UNLOCK MUTEX */


		/* state machine */
		switch(state)
		{
			case RS_UPNP_S_UNINITIALISED:
			case RS_UPNP_S_UNAVAILABLE:
				/* failed ... try again in 30 min. */
				allowedSleep = 1800;
			break;

			case RS_UPNP_S_READY:
			case RS_UPNP_S_FAILED:
			case RS_UPNP_S_ACTIVE:
				/* working ... normal 15 seconds */
				allowedSleep = 15;
			break;

			default:
				/* default??? how did it get here? */
			break;
		}

		std::cerr << "UPnPHandler::Run() sleeping for:" << allowedSleep << std::endl;
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
#ifndef WINDOWS_SYS
		sleep(allowedSleep); 
#else
		Sleep(1000 * allowedSleep); 
#endif
/********************************** WINDOWS/UNIX SPECIFIC PART ******************/
	}
	return;
}

void upnphandler::checkUPnPState()
{
	dataMtx.lock(); /* LOCK MUTEX */

	int state = upnpState;

	dataMtx.unlock(); /* UNLOCK MUTEX */

	/* state machine */
	switch(state)
	{
		case RS_UPNP_S_UNINITIALISED:
		case RS_UPNP_S_UNAVAILABLE:
			initUPnPState();
		break;

		case RS_UPNP_S_READY:
		case RS_UPNP_S_FAILED:
		case RS_UPNP_S_ACTIVE:
			printUPnPState();
			updateUPnP();
		break;

	}

	return;
}


bool upnphandler::initUPnPState()
{
	/* allocate memory */
	uPnPConfigData *upcd = new uPnPConfigData;
	int i;

	upcd->devlist = upnpDiscover(2000);
	if(upcd->devlist)
	{
		struct UPNPDev * device;
		printf("List of UPNP devices found on the network :\n");
		for(device=upcd->devlist;device;device=device->pNext)
		{
			printf("\n desc: %s\n st: %s\n",
				   device->descURL, device->st);
		}
		putchar('\n');
		if(UPNP_GetValidIGD(upcd->devlist, &(upcd->urls), 
				&(upcd->data), upcd->lanaddr, 
				sizeof(upcd->lanaddr)))
		{
			printf("Found valid IGD : %s\n", 
					upcd->urls.controlURL);
			printf("Local LAN ip address : %s\n", 
					upcd->lanaddr);

			/* MODIFY STATE */
			dataMtx.lock(); /* LOCK MUTEX */

			/* convert to ipaddress. */
			inet_aton(upcd->lanaddr, &(upnp_iaddr.sin_addr));
			upnp_iaddr.sin_port = iaddr.sin_port;

			upnpState = RS_UPNP_S_READY;
			upnpConfig = upcd;   /* */

			dataMtx.unlock(); /* UNLOCK MUTEX */




			/* done -> READY */
			return 1;
			
		}
		else
		{
			fprintf(stderr, "No valid UPNP Internet Gateway Device found.\n");
		}


		freeUPNPDevlist(upcd->devlist); 
		upcd->devlist = 0;
	}
	else
	{
		fprintf(stderr, "No IGD UPnP Device found on the network !\n");
	}

	upnpState = RS_UPNP_S_UNAVAILABLE;
	delete upcd;
	upnpConfig = NULL;

	/* done, FAILED -> NOT AVAILABLE */

	return 0;
}

bool upnphandler::printUPnPState()
{
	dataMtx.lock(); /* LOCK MUTEX */

	uPnPConfigData *config = upnpConfig;
	if ((upnpState >= RS_UPNP_S_READY) && (config))
	{
		DisplayInfos(&(config -> urls), &(config->data));
		GetConnectionStatus(&(config -> urls), &(config->data));
		ListRedirections(&(config -> urls), &(config->data));
	}
	else
	{
		std::cerr << "UPNP not Ready" << std::endl;
	}

	dataMtx.unlock(); /* UNLOCK MUTEX */

	return 1;
}



bool upnphandler::updateUPnP()
{
	dataMtx.lock(); /* LOCK MUTEX */


	uPnPConfigData *config = upnpConfig;
	if (!((upnpState >= RS_UPNP_S_READY) && (config)))
	{
		return false;
	}

	char eport1[] = "7812";
	char eport2[] = "7813";
	char eprot1[] = "TCP";
	char eprot2[] = "UDP";

	/* if we're to unload -> unload */
	if (toStop)
	{
		toStop = false;

		std::cerr << "Attempting To Remove Redirection: port: " << eport1;
		std::cerr << " Prot: " << eprot1;
		std::cerr << std::endl;

		RemoveRedirect(&(config -> urls), &(config->data), 
				eport1, eprot1);
		RemoveRedirect(&(config -> urls), &(config->data), 
				eport2, eprot2);

		upnpState = RS_UPNP_S_READY;
	}


	/* if we're to load -> load */
	if (toStart)
	{
		toStart = false;

		/* our address */

		/* our port */
		char in_addr[256];
		char in_port1[256];
		char in_port2[256];

		//struct sockaddr_in localAddr = iaddr;
		if (iaddr.sin_addr.s_addr != upnp_iaddr.sin_addr.s_addr)
		{
			std::cerr << "Warning ... Address Mismatch!";
			std::cerr << std::endl;
		}
	
		upnp_iaddr.sin_port = iaddr.sin_port;
		struct sockaddr_in localAddr = upnp_iaddr;

		snprintf(in_port1, 256, "%d", ntohs(localAddr.sin_port));
		snprintf(in_port2, 256, "%d", ntohs(localAddr.sin_port) + 1);
		snprintf(in_addr, 256, "%d.%d.%d.%d", 
			 ((localAddr.sin_addr.s_addr >> 0) & 0xff),
			 ((localAddr.sin_addr.s_addr >> 8) & 0xff),
			 ((localAddr.sin_addr.s_addr >> 16) & 0xff),
			 ((localAddr.sin_addr.s_addr >> 24) & 0xff));

		std::cerr << "Attempting Redirection: InAddr: " << in_addr;
		std::cerr << " InPort: " << in_port1;
		std::cerr << " ePort: " << eport1;
		std::cerr << " eProt: " << eprot1;
		std::cerr << std::endl;

		bool ok = true;
		ok = ok && SetRedirectAndTest(&(config -> urls), &(config->data),
				in_addr, in_port1, eport1, eprot1);
		ok = ok && SetRedirectAndTest(&(config -> urls), &(config->data),
				in_addr, in_port2, eport2, eprot2);

		/* ip port external_port protocol */
		if (ok)
		{
			upnpState = RS_UPNP_S_ACTIVE;
		}
		else
		{
			upnpState = RS_UPNP_S_FAILED;
		}
	}

	dataMtx.unlock(); /* UNLOCK MUTEX */


	return 1;

}



#ifdef USE_OLDOLD

/**************************** RS - UPNP interface ******************/

/* sample upnp client program */
int main(int argc, char ** argv)
{
	char command = 0;
	struct UPNPDev * devlist;
	char lanaddr[16];	/* my ip address on the LAN */
	int i;

#ifdef WIN32
	WSADATA wsaData;
	int nResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(nResult != NO_ERROR)
	{
		fprintf(stderr, "WSAStartup() failed.\n");
		return -1;
	}
#endif
    printf("upnpc : miniupnp test client. (c) 2006 Thomas Bernard\n");
    printf("Go to http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/\n"
	       "for more information.\n");
	if(argc>=2 && (argv[1][0] == '-'))
		command = argv[1][1];

	if(!command || argc<2 || (command == 'a' && argc<6)
	   || (command == 'd' && argc<4)
	   || (command == 'r' && argc<4))
	{
		fprintf(stderr, "Usage :\t%s -a ip port external_port protocol\tAdd port redirection\n", argv[0]);
		fprintf(stderr, "       \t%s -d external_port protocol\tDelete port redirection\n", argv[0]);
		fprintf(stderr, "       \t%s -s\t\t\t\tGet Connection status\n", argv[0]);
		fprintf(stderr, "       \t%s -l\t\t\t\tList redirections\n", argv[0]);
		fprintf(stderr, "       \t%s -r port1 protocol1 [port2 protocol2] [...]\n\t\t\t\tAdd all redirections to the current host\n", argv[0]);
		fprintf(stderr, "\nprotocol is UDP or TCP\n");
		return 1;
	}
	
	devlist = upnpDiscover(2000);
	if(devlist)
	{
		struct UPNPDev * device;
		struct UPNPUrls urls;
		struct IGDdatas data;
		printf("List of UPNP devices found on the network :\n");
		for(device = devlist; device; device = device->pNext)
		{
			printf("\n desc: %s\n st: %s\n",
				   device->descURL, device->st);
		}
		putchar('\n');
		if(UPNP_GetValidIGD(devlist, &urls, &data, lanaddr, sizeof(lanaddr)))
		{
			printf("Found valid IGD : %s\n", urls.controlURL);
			printf("Local LAN ip address : %s\n", lanaddr);
			#if 0
			printf("getting \"%s\"\n", urls.ipcondescURL);
			descXML = miniwget(urls.ipcondescURL, &descXMLsize);
			if(descXML)
			{
				/*fwrite(descXML, 1, descXMLsize, stdout);*/
				free(descXML); descXML = NULL;
			}
			#endif

			switch(command)
			{
			case 'l':
				DisplayInfos(&urls, &data);
				ListRedirections(&urls, &data);
				break;
			case 'a':
				SetRedirectAndTest(&urls, &data, argv[2], argv[3], argv[4], argv[5]);
				break;
			case 'd':
				RemoveRedirect(&urls, &data, argv[2], argv[3]);
				break;
			case 's':
				GetConnectionStatus(&urls, &data);
				break;
			case 'r':
				for(i=2; i<argc-1; i+=2)
				{
					/*printf("port %s protocol %s\n", argv[i], argv[i+1]);*/
					SetRedirectAndTest(&urls, &data,
					                   lanaddr, argv[i], argv[i], argv[i+1]);
				}
				break;
			default:
				fprintf(stderr, "Unknown switch -%c\n", command);
			}

			FreeUPNPUrls(&urls);
		}
		else
		{
			fprintf(stderr, "No valid UPNP Internet Gateway Device found.\n");
		}
		freeUPNPDevlist(devlist); devlist = 0;
	}
	else
	{
		fprintf(stderr, "No IGD UPnP Device found on the network !\n");
	}

	/*puts("************* HOP ***************");*/

	return 0;
}




#endif /* USE_OLDOLD */


