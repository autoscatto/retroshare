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

#ifndef _FORUMSDIALOG_H
#define _FORUMSDIALOG_H

#include "mainpage.h"
#include "RsAutoUpdatePage.h"
#include "ui_ForumsDialog.h"

class ForumInfo;

class ForumsDialog : public RsAutoUpdatePage 
{
  Q_OBJECT

public:
    ForumsDialog(QWidget *parent = 0);
    ~ForumsDialog();

    /* overloaded from RsAuthUpdatePage */
    virtual void updateDisplay();

private slots:
    /** Create the context popup menu and it's submenus */
    void forumListCustomPopupMenu( QPoint point );
    void threadListCustomPopupMenu( QPoint point );
    void restoreForumKeys();
    void newforum();

    void changedForum(const QString &id);
    void changedThread();
    void clickedThread (QTreeWidgetItem *item, int column);

    void replytomessage();
    //void print();
    //void printpreview();

    //void removemessage();
    void markMsgAsRead();
    void markMsgAsReadChildren();
    void markMsgAsReadAll();
    void markMsgAsUnread();
    void markMsgAsUnreadAll();
    void markMsgAsUnreadChildren();

    /* handle splitter */
    void togglethreadview();

    void createthread();
    void createmessage();

    void subscribeToForum();
    void unsubscribeToForum();

    void showForumDetails();
    void editForumDetails();

    void previousMessage ();
    void nextMessage ();

    void changedViewBox();

    void filterColumnChanged();
    void filterRegExpChanged();
    void clearFilter();

private:
    void insertForums();
    void insertThreads();
    void insertPost();
    void updateMessageSummaryList(std::string forumId);
    void forumInfoToGroupItemInfo(const ForumInfo &forumInfo, GroupItemInfo &groupItemInfo);

    void forumSubscribe(bool subscribe);
    void FillThreads(QList<QTreeWidgetItem *> &ThreadList, bool bExpandNewMessages, std::list<QTreeWidgetItem*> &itemToExpand);
    void FillChildren(QTreeWidgetItem *Parent, QTreeWidgetItem *NewParent, bool bExpandNewMessages, std::list<QTreeWidgetItem*> &itemToExpand);

    int getSelectedMsgCount(QList<QTreeWidgetItem*> *pRows, QList<QTreeWidgetItem*> *pRowsRead, QList<QTreeWidgetItem*> *pRowsUnread);
    void setMsgAsReadUnread(QList<QTreeWidgetItem*> &Rows, bool bRead);
    void markMsgAsReadUnread(bool bRead, bool bChildren, bool bForum);
    void CalculateIconsAndFonts(QTreeWidgetItem *pItem = NULL);
    void CalculateIconsAndFonts(QTreeWidgetItem *pItem, bool &bHasReadChilddren, bool &bHasUnreadChilddren);

    void processSettings(bool bLoad);
    void togglethreadview_internal();

    void FilterItems();
    bool FilterItem(QTreeWidgetItem *pItem, QString &sPattern, int nFilterColumn);

    bool m_bProcessSettings;

    QTreeWidgetItem *yourForums;
    QTreeWidgetItem *subscribedForums;
    QTreeWidgetItem *popularForums;
    QTreeWidgetItem *otherForums;

    std::string mCurrForumId;
    std::string mCurrThreadId;
    bool m_bIsForumSubscribed;
    bool m_bIsForumAdmin;

    QFont m_ForumNameFont;
    int m_LastViewType;
    std::string m_LastForumID;

    /** Qt Designer generated object */
    Ui::ForumsDialog ui;
};

#endif

