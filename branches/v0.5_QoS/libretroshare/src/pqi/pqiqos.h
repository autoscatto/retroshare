/*
 * libretroshare/src/pqi: pqiqos.h
 *
 * 3P/PQI network interface for RetroShare.
 *
 * Copyright 2011-2011 by Cyril Soler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "csoler@users.sourceforge.net"
 *
 */

// This class handles the prioritisation of RsItem, based on the 
// priority level. The QoS algorithm must ensure that:
//
// - lower priority items get out with lower rate than high priority items
// - items of equal priority get out of the queue in the same order than they got in
// - items of level n+1 are output \alpha times more often than items of level n. 
//   \alpha is a constant that is not necessarily an integer, but strictly > 1.
// - the set of possible priority levels is finite, and pre-determined.
//
// We setup packet priority according to the following table:
//   - some RsItems are omitted. The priority is the one of the closest parent item.
//   - I listed only items that are sent to friends.
//
//  Item priorities are all defined in serializer/itempriorities.h
//
//    +-------------------------+--------------------------------------------------+-----------------------------------------------+
//    |Item type                | low priority <---- --------------> high priority | Comment                                       |
//    |                         |                                                  |                                               |
//    +-------------------------+- 0 -- 1 -- 2 -- 3 -- 4 -- 5 -- 6 -- 7 -- 8 -- 9 -+-----------------------------------------------+
//    |Turtle items (Direct)    |                                                  |                                               |
//    |   RsTunnelRequestItem   |                                X                 |                                               |
//    |   RsTunnelOkItem        |                                X                 |                                               |
//    |   RsTurtleFileReqItem   |                           X                      |                                               |
//    |   RsTurtleSearchRequest |                           X                      |                                               |
//    |   RsTurtleFileDataItem  |                 X                                |                                               |
//    |   RsTurtleSearchResult  |                 X                                |                                               |
//    |   RsGenericTunnelsItem  |                 X                                | States for all turtle FT except file requests |
//    |                         |                                                  |                                               |
//    |Turtle items (Forwarded) |                                                  |                                               |
//    |   RsTurtleFileDataItem  |            X                                     |                                               |
//    |                         |                                                  |                                               |
//    |Direct file transfer     |                                                  |                                               |
//    |   RsFileRequest         |                           X                      |                                               |
//    |   RsFileChunkMapRequest |                           X                      |                                               |
//    |   RsFileCRC32MapRequest |                           X                      |                                               |
//    |   RsFileChunkMap        |                 X                                |                                               |
//    |   RsFileCRC32Map        |                 X                                |                                               |
//    |   RsFileData            |                 X                                |                                               |
//    |   RsCacheRequest        |                      X                           |                                               |
//    |   RsCacheItem           |                 X                                |                                               |
//    |                         |                                                  |                                               |
//    |Discovery                |                                                  |                                               |
//    |   RsDiscReply           |       X                                          |                                               |
//    |   RsDiscAskInfo         |            X                                     |                                               |
//    |   RsDiscVersion         |       X                                          |                                               |
//    |   RsDiscHeartBeat       |                                          X       |                                               |
//    |                         |                                                  |                                               |
//    |Chat/Msgs                |                                                  |                                               |
//    |   RsChatItem            |                                     X            | Parent of all chat messages                   |
//    |   RsChatAvatarItem      |            X                                     |                                               |
//    |   RsMsgItem             |            X                                     |                                               |
//    |                         |                                                  |                                               |
//    |Various                  |                                                  |                                               |
//    |   RsStatusItem          |            X                                     |                                               |
//    |   VOIP stream items     |                                               X  |                                               |
//    |                         |                                                  |                                               |
//    +-------------------------+--------------------------------------------------+-----------------------------------------------+
//
#include <vector>
#include <list>

class RsItem ;

class pqiQoS
{
	public:
		pqiQoS(uint32_t max_levels,float alpha) ;

		class ItemQueue 
		{
			public:
				RsItem *pop() 
				{
					if(_items.empty())
						return NULL ;

					RsItem *item = _items.front() ;
					_items.pop_front() ;

					return item ;
				}

				void push(RsItem *item) 
				{
					_items.push_back(item) ;
				}

				float _threshold ;
				float _counter ;
				float _inc ;
				std::list<RsItem*> _items ;
		};

		// This function pops items from the queue, y order of priority
		//
		RsItem *out_rsItem() ;

		// This function is used to queue items.
		//
		void in_rsItem(RsItem *item) ;

		void print() const ;
		uint64_t qos_queue_size() const { return _nb_items ; }

	private:
		// This vector stores the lists of items with equal priorities.
		//
		std::vector<ItemQueue> _item_queues ;
		float _alpha ;
		uint64_t _nb_items ;
};


