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

#ifndef GCONFIG_H
#define GCONFIG_H

#include <QtCore/QtCore>

#define GDOMAIN "Goatsoft"
#define GPRODUCT "Gloster2"

//Forward declaration.
class QWidget;
class QTableWidget;
class QToolBar;
class QMainWindow;

/*!	\class GConfig
	\brief The Gloster configuration class. It extends the basic functionalities
	provided by QSettings.

	All the settings are stored in an INI file in a specific directory depending on the platform :

	- Windows : "\%HOME\Application data\Rshare\config.ini".
	- Linux : "%HOME\.Rshare\config.ini"

*/

class GConfig: private QSettings
{
 Q_OBJECT

 public:
        //! A GConfig constructor.
        /*!
           Create a GConfig and load everything.
        */
        GConfig();

        //! The GConfig destuctor.
        ~GConfig();
        
        //! Save all settings on the disk.
        /*!
           Normally, all settings are written on changes but, to be sure, this
           method is called on quitting the application. 
        */
        void save();

#if !defined(G_NO_GUI)
        //! Save placement, state and size information of a window.
        void saveWidgetInformation(QWidget *widget);
        
        //! Load placement, state and size information of a window.
        void loadWidgetInformation(QWidget *widget);
        


        //! Method overload. Save window and toolbar information.
        /*!
          Save placement, state and size information of a window.
          Save the toolbararea attribute of the toolbar.
          Must be called with the widget containing the toolbar as parameter.
        */
        void saveWidgetInformation(QMainWindow *widget, QToolBar *toolBar);

        //! Method overload. Restore window and toolbar information.
        /*!
          Load placement, state and size information of a window.
          Load the toolbararea attribute of the toolbar.
          Must be called with the widget containing the toolbar as parameter.
        */
        void loadWidgetInformation(QMainWindow *widget, QToolBar *toolBar);
#endif // !G_NO_GUI
};

#endif
