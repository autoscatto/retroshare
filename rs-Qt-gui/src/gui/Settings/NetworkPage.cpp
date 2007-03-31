/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2006, crypton
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

#include "NetworkPage.h"
#include "config/gconfig.h"
#include "rshare.h"

NetworkPage::NetworkPage(QWidget * parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
    ui.setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(windowTitle() + QLatin1String(" - Gloster 2"));

    GConfig config;
    config.loadWidgetInformation(this);
    
     /* Create RshareSettings object */
   _settings = new RshareSettings();

 
}

void
NetworkPage::closeEvent (QCloseEvent * event)
{
    GConfig config;
    config.saveWidgetInformation(this);

    QWidget::closeEvent(event);
}


/** Saves the changes on this page */
bool
NetworkPage::save(QString &errmsg)
{

}
  
/** Loads the settings for this page */
void
NetworkPage::load()
{

}

