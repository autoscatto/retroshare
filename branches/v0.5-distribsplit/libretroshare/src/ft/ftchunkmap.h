#pragma once

#include <map>
#include "retroshare/rstypes.h"

// ftChunkMap: 
// 	- handles chunk map over a complete file
// 	- mark down which chunk is being downloaded by which peer
// 	- allocate data ranges of any requested size for a given peer
// 		- continuing an existing chunk
// 		- allocating a new chunk
//
// Download mecanism:
// 	- ftFileCreator handles a list of active slices, and periodically resends requests every 20 sec.
// 		Slices have arbitrary size (less than a chunk), depending on the transfer rate.
// 		When receiving data, ftFileCreator shrinks its slices until they get complete. When a slice is finished, it
// 		notifies ftChunkMap that this slice is done.
//
// 	- ftChunkMap maintains two levels:
// 		- the chunk level (Chunks a 1MB long) with a map of who has which chunk and what locally is the state of 
// 		each chunk
// 		- the slice level: each active chunk is cut into slices (basically a list of intervalls) being downloaded, and
// 		a remaining slice to cut off new candidates. When notified for a complete slice, ftChunkMap removed the
// 		corresponding acive slice. When asked a slice, ftChunkMap cuts out a slice from the remaining part of the chunk
// 		to download, sends the slice's coordinates and gives a unique slice id (such as the slice offset).


// This class handles a slice of a chunk of arbitrary uint32_t size, at the level of ftFileCreator

class ftController ;

class ftChunk 
{
	public:
		typedef uint64_t ChunkId ;

		ftChunk():offset(0), size(0), ts(0) {}

		friend std::ostream& operator<<(std::ostream& o,const ftChunk& f) ;

		uint64_t offset;	// current offset of the slice
		uint64_t size;		// size remaining to download
		ChunkId  id ;		// id of the chunk. Equal to the starting offset of the chunk
		time_t   ts;		// time of last data received
		std::string peer_id ;
};

// This class handles a single fixed-sized chunk. Although each chunk is requested at once,
// it may be sent back into sub-chunks because of file transfer rate constraints. 
// So the dataReceived function should be called to progressively complete the chunk,
// and the getChunk method should ask for a sub-chunk of a given size.
//
class Chunk
{
	public: 
		Chunk(): _start(0),_offset(0),_end(0) {}	// only used in default std::map fillers

		Chunk(uint64_t start,uint32_t size) ;

		void getSlice(uint32_t size_hint,ftChunk& chunk) ;

		// Returns true when the chunk is complete
		bool empty() const { return _offset == _end ; }

		// Array of intervalls of bytes to download.
		//
		uint64_t _start ;		// const
		uint64_t _offset ;	// not const: handles the current offset within the chunk.
		uint64_t _end ;		// const
};

class ChunkDownloadInfo
{
	public:
		std::map<ftChunk::ChunkId,uint32_t> _slices ;
		uint32_t _remains ;
		time_t _last_data_received ;
};

class SourceChunksInfo
{
	public:
		CompressedChunkMap cmap ;	//! map of what the peer has/doens't have
		time_t TS ;						//! last update time for this info
		bool is_full ;					//! is the map full ? In such a case, re-asking for it is unnecessary.
};

class ChunkMap
{
   public:
		static const uint32_t CHUNKMAP_FIXED_CHUNK_SIZE = 1024*1024 ; // 1 MB chunk
		typedef uint32_t ChunkNumber ;

		/// Constructor. Decides what will be the size of chunks and how many there will be.

		ChunkMap(uint64_t file_size,bool assume_availability) ;

		/// destructor
		virtual ~ChunkMap() {}

      /// Returns an slice of data to be asked to the peer within a chunk.
		/// If a chunk is already been downloaded by this peer, take a slice at
		/// the beginning of this chunk, or at least where it starts.
		/// If not, randomly/streamly select a new chunk depending on the strategy. 
      /// adds an entry in the chunk_ids map, and sets up 1 interval for it.
      /// the chunk should be available from the designated peer. 
		/// On return:
		/// 	- source_chunk_map_needed 	= true if the source map should be asked

      virtual bool getDataChunk(const std::string& peer_id,uint32_t size_hint,ftChunk& chunk,bool& source_chunk_map_needed) ; 

      /// Notify received a slice of data. This needs to 
      ///   - carve in the map of chunks what is received, what is not.
      ///   - tell which chunks are finished. For this, each interval must know what chunk number it has been attributed
      ///    when the interval is split in the middle, the number of intervals for the chunk is increased. If the interval is
      ///    completely covered by the data, the interval number is decreased.

      virtual void dataReceived(const ftChunk::ChunkId& c_id) ;

      /// Decides how chunks are selected. 
      ///    STREAMING: the 1st chunk is always returned
      ///       RANDOM: a uniformly random chunk is selected among available chunks for the current source.
      ///              

		void setStrategy(FileChunksInfo::ChunkStrategy s) { _strategy = s ; }
		FileChunksInfo::ChunkStrategy getStrategy() const { return _strategy ; }

      /// Properly fills an vector of fixed size chunks with availability or download state.
      /// chunks is given with the proper number of chunks and we have to adapt to it. This can be used
      /// to display square chunks in the gui or display a blue bar of availability by collapsing info from all peers.
		/// The set method is not virtual because it has no reason to exist in the parent ftFileProvider

      virtual void getAvailabilityMap(CompressedChunkMap& cmap) const ;
		void setAvailabilityMap(const CompressedChunkMap& cmap) ;

		/// Removes the source availability map. The map
		void removeFileSource(const std::string& peer_id) ;

		/// This function fills in a plain map for a file of the given size. This
		/// is used to ensure that the chunk size will be consistent with the rest
		/// of the code.
		//
		static void buildPlainMap(uint64_t size,CompressedChunkMap& map) ;

		/// Computes the number of chunks for the given file size.
		static uint32_t getNumberOfChunks(uint64_t size) ;
		
		/// This function is used by the parent ftFileProvider to know whether the chunk can be sent or not.
		bool isChunkAvailable(uint64_t offset, uint32_t chunk_size) const ;

		/// Remove active chunks that have not received any data for the last 60 seconds, and return
		/// the list of slice numbers that should be canceled.
		void removeInactiveChunks(std::vector<ftChunk::ChunkId>& to_remove) ;

		/// Updates the peer's availablility map
		//
		void setPeerAvailabilityMap(const std::string& peer_id,const CompressedChunkMap& peer_map) ;

		/// Returns the total size of downloaded data in the file.
		uint64_t getTotalReceived() const { return _total_downloaded ; }

		/// returns true is the file is complete
		bool isComplete() const { return _file_is_complete ; }

		void getChunksInfo(FileChunksInfo& info) const ;

	protected:
		/// handles what size the last chunk has.
		uint32_t sizeOfChunk(uint32_t chunk_number) const ;

		/// Returns a chunk available for this peer_id, depending on the chunk strategy.
		//
		uint32_t getAvailableChunk(const std::string& peer_id,bool& chunk_map_too_old) ;

	private:
		uint64_t												_file_size ;						//! total size of the file in bytes.
		uint32_t												_chunk_size ;						//! Size of chunks. Common to all chunks.
		FileChunksInfo::ChunkStrategy 				_strategy ;							//! how do we allocate new chunks
		std::map<std::string,Chunk>					_active_chunks_feed ; 			//! vector of chunks being downloaded. Exactly 1 chunk per peer.
		std::map<ChunkNumber,ChunkDownloadInfo>	_slices_to_download ; 			//! list of (slice id,slice size) 
		std::vector<FileChunksInfo::ChunkState>	_map ;								//! vector of chunk state over the whole file
		std::map<std::string,SourceChunksInfo>		_peers_chunks_availability ;	//! what does each source peer have
		uint64_t												_total_downloaded ;				//! completion for the file
		bool													_file_is_complete ;           //! set to true when the file is complete.
		bool													_assume_availability ;			//! true if all sources always have the complete file.
};


