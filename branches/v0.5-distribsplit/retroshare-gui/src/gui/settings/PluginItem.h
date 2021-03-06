/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2011 Cyril Soler  
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

#pragma once

#include "ui_PluginItem.h"

class PluginItem: public QWidget, public Ui::PluginItem
{
	Q_OBJECT

	public:
		PluginItem(int id,const QString& pluginTitle,const QString& pluginDescription,const QString& status, const QString& file_name, const QString& file_hash, const QString& error_string, const QIcon& icon) ;

	protected slots:
		void togglePlugin(bool) ;
		void configurePlugin() ;

	signals:
		void pluginEnabled(bool,const QString&) ;
		void pluginConfigure(int) ;

	private:
		int _id ;
};


