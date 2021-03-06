/*
 * ChanGroupDelegate.cpp
 *
 *  Created on: Sep 7, 2009
 *      Author: alex
 */

#include "ChanGroupDelegate.h"
#include <QApplication>
#include <QPainter>
#include <QColor>

void ChanGroupDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if (index.child(0, 0).isValid()) {
		painter->setPen(Qt::blue);
		QStyleOptionButton opt;
		opt.rect = option.rect;
		QApplication::style()->drawControl(QStyle::CE_PushButtonBevel, &opt, painter);
	}

	QItemDelegate::paint(painter, option, index);
}
