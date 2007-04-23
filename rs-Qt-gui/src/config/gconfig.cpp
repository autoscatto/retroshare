/****************************************************************
 *  Retroshare Qt gui is distributed under the following license:
 *
 *  Copyright (C) 2006,  crypton
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

#include <QtCore/QtCore>
#include <QWidget>
#include <QTableWidget>
#include <QMainWindow>
#include "gconfig.h"

GConfig::GConfig()
  : QSettings(QSettings::IniFormat, QSettings::UserScope, GDOMAIN, GPRODUCT)
{
#if !defined(Q_WS_WIN)
    setPath(QSettings::IniFormat, QSettings::UserScope, "$HOME/.Rshare");
#endif
}

GConfig::~GConfig()
{

}

void
GConfig::save()
{

}

//From here you can find methods compiled only if the GUI is not disabled
#if !defined(G_NO_GUI)

void
GConfig::saveWidgetInformation(QWidget *widget)
{
    beginGroup("widgetInformation");
    beginGroup(widget->objectName());

    setValue("size", widget->size());
    setValue("pos", widget->pos());

    endGroup();
    endGroup();
}

void
GConfig::saveWidgetInformation(QMainWindow *widget, QToolBar *toolBar)
{
 beginGroup("widgetInformation");
 beginGroup(widget->objectName());

 setValue("toolBarArea", widget->toolBarArea(toolBar));

 endGroup();
 endGroup();
 
 saveWidgetInformation(widget);
}

void
GConfig::loadWidgetInformation(QWidget *widget)
{
 beginGroup("widgetInformation");
 beginGroup(widget->objectName());
 
 widget->resize(value("size", widget->size()).toSize());
 widget->move(value("pos", QPoint(200, 200)).toPoint());

 endGroup();
 endGroup();
}

void
GConfig::loadWidgetInformation(QMainWindow *widget, QToolBar *toolBar)
{
 beginGroup("widgetInformation");
 beginGroup(widget->objectName());
 
 widget->addToolBar((Qt::ToolBarArea) value("toolBarArea", Qt::TopToolBarArea).toInt(),
                    toolBar);
 
 endGroup();
 endGroup();
 
 loadWidgetInformation(widget);
}



#endif // !G_NO_GUI
