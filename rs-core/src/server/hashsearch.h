#ifndef MRK_FILE_HASH_SEARCH_H
#define MRK_FILE_HASH_SEARCH_H

/**********
 * SearchInterface for the FileTransfer
 */

#include "rsiface/rstypes.h"

class FileHashSearch
{
	public:
	FileHashSearch();
virtual ~FileHashSearch();

	/* Search Interface - For FileTransfer Lookup */
virtual	int searchHash(std::string id, std::string hash, std::list<FileDetail> &results) = 0;

};

#endif
