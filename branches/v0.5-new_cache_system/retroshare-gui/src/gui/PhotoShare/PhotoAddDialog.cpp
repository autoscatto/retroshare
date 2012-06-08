/*
 * Retroshare Photo Plugin.
 *
 * Copyright 2012-2012 by Robert Fernie.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2.1 as published by the Free Software Foundation.
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

#include "gui/PhotoShare/PhotoAddDialog.h"
#include "gui/PhotoShare/PhotoDetailsDialog.h"
#include "gui/PhotoShare/PhotoDrop.h"

#include <iostream>

/** Constructor */
PhotoAddDialog::PhotoAddDialog(QWidget *parent)
: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.scrollAreaWidgetContents, SIGNAL( buttonStatus( uint32_t ) ), this, SLOT( updateMoveButtons( uint32_t ) ) );
	connect(ui.pushButton_ShiftLeft, SIGNAL( clicked( void ) ), ui.scrollAreaWidgetContents, SLOT( moveLeft( void ) ) );
	connect(ui.pushButton_ShiftRight, SIGNAL( clicked( void ) ), ui.scrollAreaWidgetContents, SLOT( moveRight( void ) ) );
	connect(ui.pushButton_EditPhotoDetails, SIGNAL( clicked( void ) ), this, SLOT( showPhotoDetails( void ) ) );

	connect(ui.pushButton_Publish, SIGNAL( clicked( void ) ), this, SLOT( publishAlbum( void ) ) );

	mPhotoDetails = NULL;

}


void PhotoAddDialog::updateMoveButtons(uint32_t status)
{
        std::cerr << "PhotoAddDialog::updateMoveButtons(" << status << ")";
        std::cerr << std::endl;

	switch(status)
	{
		case PHOTO_SHIFT_NO_BUTTONS:
                	ui.pushButton_ShiftLeft->setEnabled(false);
                	ui.pushButton_ShiftRight->setEnabled(false);
			break;
		case PHOTO_SHIFT_LEFT_ONLY:
                	ui.pushButton_ShiftLeft->setEnabled(true);
                	ui.pushButton_ShiftRight->setEnabled(false);
			break;
		case PHOTO_SHIFT_RIGHT_ONLY:
                	ui.pushButton_ShiftLeft->setEnabled(false);
                	ui.pushButton_ShiftRight->setEnabled(true);
			break;
		case PHOTO_SHIFT_BOTH:
                	ui.pushButton_ShiftLeft->setEnabled(true);
                	ui.pushButton_ShiftRight->setEnabled(true);
			break;
	}
}


void PhotoAddDialog::showPhotoDetails()
{
        std::cerr << "PhotoAddDialog::showPhotoDetails()";
        std::cerr << std::endl;

	if (!mPhotoDetails)
	{
		mPhotoDetails = new PhotoDetailsDialog(NULL);
	}

	PhotoItem *item = ui.scrollAreaWidgetContents->getSelectedPhotoItem();

	mPhotoDetails->setPhotoItem(item);
	mPhotoDetails->show();
}




void PhotoAddDialog::publishAlbum()
{
        std::cerr << "PhotoAddDialog::publishAlbum()";
        std::cerr << std::endl;

	/* we need to iterate through each photoItem, and extract the details */


	RsPhotoAlbum album;

	album.mShareOptions.mShareType = 0;
	album.mShareOptions.mShareGroupId = "unknown";
	album.mShareOptions.mPublishKey = "unknown";
	album.mShareOptions.mCommentMode = 0;
	album.mShareOptions.mResizeMode = 0;

	album.mTitle = ui.lineEdit_Title->text().toStdString();
	album.mCategory = "Unknown";
	album.mCaption = ui.lineEdit_Caption->text().toStdString();
	album.mWhere = ui.lineEdit_Where->text().toStdString();
	album.mWhen = ui.lineEdit_When->text().toStdString();

	if (rsPhoto->submitAlbumDetails(album))
	{
		/* now have path and album id */
		int photoCount = ui.scrollAreaWidgetContents->getPhotoCount();

		for(int i = 0; i < photoCount; i++)
		{
			RsPhotoPhoto photo;
			PhotoItem *item = ui.scrollAreaWidgetContents->getPhotoIdx(i);
			photo = item->mDetails;
			item->getPhotoThumbnail(photo.mThumbnail);
	
			photo.mAlbumId = album.mAlbumId;
			photo.mOrder = i;

			/* scale photo if needed */
			if (album.mShareOptions.mResizeMode)
			{
				/* */

			}
			/* save image to album path */
			photo.path = "unknown";

			rsPhoto->submitPhoto(photo);
		}
	}

	clearDialog();

	hide();
}


void PhotoAddDialog::clearDialog()
{
	ui.lineEdit_Title->setText(QString("title"));
	ui.lineEdit_Caption->setText(QString("Caption"));
	ui.lineEdit_Where->setText(QString("Where"));
	ui.lineEdit_When->setText(QString("When"));

	ui.scrollAreaWidgetContents->clearPhotos();
}

	
