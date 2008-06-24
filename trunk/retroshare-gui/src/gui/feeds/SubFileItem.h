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

#ifndef _SUB_FILE_ITEM_DIALOG_H
#define _SUB_FILE_ITEM_DIALOG_H

#include "ui_SubFileItem.h"

#include <string>

class SubFileItem : public QWidget, private Ui::SubFileItem
{
  Q_OBJECT

public:
  	/** Default Constructor */
  	SubFileItem(std::string hash, std::string name, uint64_t size);

  	/** Default Destructor */

  	void small();
  	bool done();

	std::string FileHash() { return mFileHash; }
	std::string FileName() { return mFileName; }
	uint64_t    FileSize() { return mFileSize; }

void updateItemStatic();

private slots:
	/* default stuff */
  	void cancel();
	void play();
	void toggle();

	void updateItem();

private:
	std::string mFileHash;
	std::string mFileName;
	uint64_t    mFileSize;

	/* for display purposes */
	float amountDone;
};



#endif

