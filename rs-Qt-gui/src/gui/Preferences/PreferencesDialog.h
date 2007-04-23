/****************************************************************
 *  RShare is distributed under the following license:
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

#ifndef _PREFERENCESDIALOG_H
#define _PREFERENCESDIALOG_H
#include <QtGui>
#include <QFileDialog>
#include <QStyleFactory>
#include <QLineEdit>

#include <config/rsharesettings.h>
#include <lang/languagesupport.h>

#include "configpage.h"
#include "ui_PreferencesDialog.h"

class PreferencesDialog : public ConfigPage 
{
  Q_OBJECT

public:
  /** Default Constructor */
  PreferencesDialog(QWidget *parent = 0);
  /** Default Destructor */ 
  ~PreferencesDialog();
  /** Saves the changes on this page */
  bool save(QString &errmsg);
  /** Loads the settings for this page */
  void load();


private slots:

  void on_styleSheetCombo_activated(const QString &styleSheetName);

  
private:
  /** A VidaliaSettings object used for saving/loading settings */
  RshareSettings* _settings;
  
  void loadStyleSheet(const QString &sheetName);
  



  /** Qt Designer generated object */
  Ui::PreferencesDialog ui;
};

#endif

