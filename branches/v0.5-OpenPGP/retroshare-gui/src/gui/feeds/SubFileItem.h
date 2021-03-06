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
#include <stdint.h>
const uint32_t SFI_MASK_STATE		= 0x000f;
const uint32_t SFI_MASK_TYPE		= 0x00f0;
//const uint32_t SFI_MASK_FT		= 0x0f00;
const uint32_t SFI_MASK_FLAG		= 0xf000;

const uint32_t SFI_STATE_ERROR		= 0x0001;
const uint32_t SFI_STATE_EXTRA		= 0x0002;
const uint32_t SFI_STATE_REMOTE		= 0x0003;
const uint32_t SFI_STATE_DOWNLOAD	= 0x0004;
const uint32_t SFI_STATE_LOCAL		= 0x0005;
const uint32_t SFI_STATE_UPLOAD		= 0x0006;

const uint32_t SFI_TYPE_CHANNEL		= 0x0010;
const uint32_t SFI_TYPE_ATTACH		= 0x0020;

const uint32_t SFI_FLAG_CREATE		= 0x1000;


//! This create a gui widget that allows users to access files shared by user
/*!
 * Widget that allows user to share files with a visual attachment interface
 * Note: extra files (files not already shared/hashed in rs) need to
 * be hashed by the clients of this class or else objects of this class will
 * have reduced functionality
 */
class SubFileItem : public QWidget, private Ui::SubFileItem
{
	Q_OBJECT

public:
	/** Default Constructor */
	SubFileItem(const std::string &hash, const std::string &name, const std::string &path, uint64_t size, uint32_t flags, const std::string &srcId);

	void smaller();

	std::string FileHash() { return mFileHash; }
	std::string FileName() { return mFileName; }
	uint64_t    FileSize() { return mFileSize; }
	std::string FilePath() { return mPath; }

	void updateItemStatic();

	bool done();
	bool ready();
	uint32_t getState();

	bool isDownloadable(bool &startable);
	bool isPlayable(bool &startable);

public slots:
	void download();
	void play();

private slots:
	void toggle();

  	void cancel();
	void save();

	void updateItem();

private:
	void Setup();

	std::string mPath;
	std::string mFileHash;
	std::string mFileName;
	uint64_t    mFileSize;
	std::string mSrcId;

	uint32_t    mMode;
	uint32_t    mType;
	uint32_t    mFlag;
	uint64_t    mDivisor;

	/* for display purposes */
	float amountDone;

signals:
	void fileFinished(SubFileItem * subFileItem);
};

#endif
