/****************************************************************
 *
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2012, RetroShare Team
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

#ifndef FRIENDSELECTIONWIDGET_H
#define FRIENDSELECTIONWIDGET_H

#include <QWidget>

namespace Ui {
class FriendSelectionWidget;
}

class QTreeWidgetItem;
class RSTreeWidgetItemCompareRole;

class FriendSelectionWidget : public QWidget
{
	Q_OBJECT

	Q_PROPERTY(QColor textColorOnline READ textColorOnline WRITE setTextColorOnline)

public:
	enum IdType
	{
		IDTYPE_NONE,
		IDTYPE_GROUP,
		IDTYPE_SSL,
		IDTYPE_GPG
	};

	enum Modus
	{
		MODUS_SINGLE,
		MODUS_MULTI,
		MODUS_CHECK
	};

    enum ShowType {
        SHOW_GROUP = 1,
        SHOW_GPG   = 2,
        SHOW_SSL   = 4
    };

    Q_DECLARE_FLAGS(ShowTypes, ShowType)

public:
	explicit FriendSelectionWidget(QWidget *parent = 0);
	~FriendSelectionWidget();

	void setHeaderText(const QString &text);
	void setModus(Modus modus);
	void setShowType(ShowTypes types);
	void start();

	int selectedItemCount();
	QString selectedId(IdType &idType);
	void selectedSslIds(std::list<std::string> &sslIds, bool onlyDirectSelected) { selectedIds(IDTYPE_SSL, sslIds, onlyDirectSelected); }
	void selectedGpgIds(std::list<std::string> &gpgIds, bool onlyDirectSelected) { selectedIds(IDTYPE_GPG, gpgIds, onlyDirectSelected); }
	void selectedGroupIds(std::list<std::string> &groupIds) { selectedIds(IDTYPE_GROUP, groupIds, true); }

	void setSelectedSslIds(const std::list<std::string> &sslIds, bool add) { setSelectedIds(IDTYPE_SSL, sslIds, add); }
	void setSelectedGpgIds(const std::list<std::string> &gpgIds, bool add) { setSelectedIds(IDTYPE_GPG, gpgIds, add); }
	void setSelectedGroupIds(const std::list<std::string> &groupIds, bool add) { setSelectedIds(IDTYPE_GROUP, groupIds, add); }

	QColor textColorOnline() const { return mTextColorOnline; }

	void setTextColorOnline(QColor color) { mTextColorOnline = color; }

protected:
	void changeEvent(QEvent *e);

signals:
	void contentChanged();
	void customContextMenuRequested(const QPoint &pos);
	void doubleClicked(IdType idType, const QString &id);

private slots:
	void fillList();
	void peerStatusChanged(const QString& peerId, int status);
	void filterItems(const QString &text);
	void contextMenuRequested(const QPoint &pos);
	void itemDoubleClicked(QTreeWidgetItem *item, int column);
	void itemChanged(QTreeWidgetItem *item, int column);

private:
	bool filterItem(QTreeWidgetItem *item, const QString &text);

	void selectedIds(IdType idType, std::list<std::string> &ids, bool onlyDirectSelected);
	void setSelectedIds(IdType idType, const std::list<std::string> &ids, bool add);

	bool started;
	RSTreeWidgetItemCompareRole *compareRole;
	Modus listModus;
	ShowTypes showTypes;
	bool inGroupItemChanged;
	bool inGpgItemChanged;
	bool inSslItemChanged;

	/* Color definitions (for standard see qss.default) */
	QColor mTextColorOnline;

	Ui::FriendSelectionWidget *ui;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FriendSelectionWidget::ShowTypes)

#endif // FRIENDSELECTIONWIDGET_H
