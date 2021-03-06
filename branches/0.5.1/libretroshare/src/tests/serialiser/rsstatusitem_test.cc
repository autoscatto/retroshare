/*
 * libretroshare/src/tests/serialiser: rsstatusitem_test.cc
 *
 * RetroShare Serialiser.
 *
 * Copyright 2010 by Christopher Evi-Parker.
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
 * Please report all bugs and problems to "retroshare@lunamutt.com".
 *
 */


#include "support.h"
#include "rsstatusitem_test.h"

INITTEST();

RsSerialType* init_item(RsStatusItem& rsi)
{

	return new RsStatusSerialiser();
}

bool operator ==(RsStatusItem& rsi1, RsStatusItem& rsi2)
{
	return false;
}


int main()
{

	test_RsItem<RsStatusItem >(); REPORT("Serialise/Deserialise RsStatusItem");
	FINALREPORT("RsStatusItem Tests");

	return TESTRESULT();
}
