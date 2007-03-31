#ifndef RS_TYPES_GUI_INTERFACE_H
#define RS_TYPES_GUI_INTERFACE_H


#include <list>
#include <iostream>
#include <string>

#define RSCERTIDLEN 16

class RsCertId
{
        public:
	RsCertId();
	RsCertId(std::string idstr);

        bool    operator<(const RsCertId &ref) const;
        bool    operator==(const RsCertId &ref) const;
        char    data[RSCERTIDLEN];
};

std::ostream &operator<<(std::ostream &out, const RsCertId &id);

/* use RsCertId, (not unsigned long) because the definition will change 
typedef unsigned long RsCertId; 
*/

typedef RsCertId      RsChanId; 
typedef std::string   RsAuthId; 


/* forward declarations of the classes */

#define INFO_SAME 0x01
#define INFO_CHG  0x02
#define INFO_NEW  0x04
#define INFO_DEL  0x08

class BaseInfo
{
	public:
	RsCertId id; /* key for matching everything */
	int flags; /* INFO_TAG above */
};

class NeighbourInfo: public BaseInfo
{
	public:
	std::string name;
	std::string org;
	std::string loc;
	std::string country;
	int         trustLvl;
	std::string trustString;
	std::list<RsCertId> signers;
	std::string authCode;
	int status;

	std::string acceptString;
	std::string statusString;
	std::string connectString;
	std::string lastConnect;
	std::string peerAddress;

	bool ownsign;
};
	
/********************** For the Directory Listing *****************/

class FileInfo: public BaseInfo
{
	public:

static const int kRsFiStatusNone = 0;
static const int kRsFiStatusStall = 1;
static const int kRsFiStatusProgress = 2;
static const int kRsFiStatusDone = 2;

	/* FileInfo(); */

	int searchId;      /* 0 if none */
	std::string path;
	std::string fname;
	std::string hash;
	std::string ext;

	int size; 
	double rank;
	int avail; /* how much we have */
	int status;
};

class DirInfo: public BaseInfo
{
	public:
	std::string path;
	std::string dirname;
	std::list<DirInfo> subdirs;
	std::list<FileInfo> files;
	int infoAge; 
	int nofiles;
	int nobytes;
};

class PersonInfo: public BaseInfo
{
	public:
	std::string name;
	bool online;
	int infoAge; /* time() at when this was last updated */

	DirInfo rootdir;
};

/********************** For Messages and Channels *****************/

class FileTransferInfo: public FileInfo
{
	public:
	int transfered;
	bool download;
};


/********************** For Messages and Channels *****************/

class MessageInfo: public BaseInfo
{
	public:
	MessageInfo() :from(0), to(0) {}
	RsCertId from;
	RsCertId to;
	std::string title;
	std::string msg;
	std::list<FileInfo> files;
	int size;  /* total of files */
	int count; /* file count     */

	int ts;
};

class ChannelInfo: public BaseInfo
{
	public:
	ChannelInfo() :chanId(0), publisher(false) {}
	RsChanId chanId;
	bool publisher;
	std::string chanName;
	std::list<MessageInfo> msglist;

	/* details */
	int mode;
	float rank;


	int size;  /* total of msgs */
	int count; /* msg count     */
};

class ChatInfo: public BaseInfo
{
	public:
	std::string msg;
};


/********************** For Search Interface *****************/

/* This is still rough, implement later! */

	/* text based ones */
const std::string TypeExt  = "ext";
const std::string TypeName = "name";
const std::string TypeHash = "hash";
const std::string TypeSize = "size";

const int OpContains    = 0x001;
const int OpExactMatch  = 0x002;
const int OpLessThan    = 0x003;
const int OpGreaterThan = 0x004;

class Condition
{
	public:

	std::string type;
	int op;
	double value;
	std::string name;
};

class SearchRequest
{
	public:
	int searchId;	
	RsCertId toId;  /* all zeros for everyone! */
	std::list<Condition> tests;
};


std::ostream &operator<<(std::ostream &out, const NeighbourInfo &info);
std::ostream &operator<<(std::ostream &out, const MessageInfo &info);
std::ostream &operator<<(std::ostream &out, const ChannelInfo &info);
std::ostream &operator<<(std::ostream &out, const ChatInfo &info);
std::ostream &operator<<(std::ostream &out, const PersonInfo &info);
std::ostream &print(std::ostream &out, const DirInfo &info, int indentLvl);


#endif


