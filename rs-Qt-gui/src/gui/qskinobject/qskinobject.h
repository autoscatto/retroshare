/****************************************************************
 *  RShare is distributed under the following license:
 *
 *  Copyright (C) 2006, The RetroShare Team
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
#ifndef QSkinObject_H
#define QSkinObject_H
#include "qskinwidgetresizehandler.h"

#include <QtCore>
#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QMouseEvent>
#include <QDesktopWidget>
#include <QBitmap>
#include <QEvent>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QBasicTimer>
#ifdef WIN32
#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#include <windows.h>
#endif
class QSkinWidgetResizeHandler;
class  QSkinObject : public QObject
{
    Q_OBJECT
    friend class QSkinWidgetResizeHandler;
public:
    	QSkinObject(QWidget* wgtParent);
	~QSkinObject(){}
	void setSkinPath(const QString & skinpath);
	QString getSkinPath();
	int customFrameWidth();
public slots:
	void updateStyle();
	void updateButtons();
	void startSkinning();
	void stopSkinning();
protected:
	bool eventFilter(QObject *o, QEvent *e);
	//Events to filter
	//void mouseMoveEvent(QMouseEvent *event);
    	void mousePressEvent(QMouseEvent *event);
    	//void mouseReleaseEvent(QMouseEvent *mouseEvent);
    	void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *e);
	//void closeEvent(QCloseEvent *e);

	void loadSkinIni();
	void manageRegions();
	QPixmap drawCtrl(QWidget * widget);
	QRegion childRegion;
	void timerEvent ( QTimerEvent * event );
private:
    	QPoint dragPosition;
	QPixmap widgetMask;//the pixmap, in which the ready frame is stored on pressed? 
	QString skinPath;
	QFont titleFont;
	QColor titleColor;
	QColor backgroundColor;
	bool milchglas;
    	bool gotMousePress;	
	QRegion quitButton;
	QRegion maxButton;
	QRegion minButton;
	QRect contentsRect;
	QSkinWidgetResizeHandler * resizeHandler;
	bool mousePress;
	QBasicTimer *skinTimer;
	QWidget *skinWidget;
	void fastbluralpha(QImage &img, int radius);
	Qt::WindowFlags flags;
	int wlong;
#ifdef WIN32
public slots:
	void setLayered();
	void updateAlpha();
private:
	double alpha;
#endif
};
#endif

