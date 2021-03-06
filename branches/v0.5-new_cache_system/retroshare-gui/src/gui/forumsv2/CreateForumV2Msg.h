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


#ifndef _CREATE_FORUMV2_MSG_DIALOG_H
#define _CREATE_FORUMV2_MSG_DIALOG_H

#include "ui_CreateForumV2Msg.h"

class CreateForumV2Msg : public QMainWindow
{
  Q_OBJECT

public:
    CreateForumV2Msg(std::string fId, std::string pId);

    void newMsg(); /* cleanup */

private slots:
    /** Create the context popup menu and it's submenus */
    void forumMessageCostumPopupMenu( QPoint point );

    void fileHashingFinished(QList<HashedFile> hashedFiles);
    /* actions to take.... */
    void createMsg();
    void cancelMsg();
    void pasteLink();
    void pasteLinkFull();

    void smileyWidgetForums();
    void addSmileys();
    void addFile();

protected:
    void closeEvent (QCloseEvent * event);

private:
    std::string mForumId;
    std::string mParentId;

    /** Qt Designer generated object */
    Ui::CreateForumV2Msg ui;
};

#endif

