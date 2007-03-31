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



#ifndef _RSHARESETTINGS_H
#define _RSHARESETTINGS_H

#include <QSettings>

#include <gui/linetypes.h>


/** Handles saving and restoring RShares's settings, such as the
 * location of Tor, the control port, etc.
 *
 * NOTE: Qt 4.1 documentation states that constructing a QSettings object is
 * "very fast", so we shouldn't need to create a global instance of this
 * class.
 */
class RshareSettings : protected QSettings
{
  
public:
  /** Default constructor. */
  RshareSettings();

  /** Resets all of Rshare's settings. */
  static void reset();

  /** Gets the currently preferred language code for RShare. */
  QString getLanguageCode();
  /** Saves the preferred language code. */
  void setLanguageCode(QString languageCode);
 
  /** Gets the interface style key (e.g., "windows", "motif", etc.) */
  QString getInterfaceStyle();
  /** Sets the interface style key. */
  void setInterfaceStyle(QString styleKey);
  
    /* Get the destination log file. */
  QString getLogFile();
  /** Set the destination log file. */
  void setLogFile(QString file);

  /* Get the bandwidth graph line filter. */
  uint getBWGraphFilter();
  /** Set the bandwidth graph line filter. */
  void setBWGraphFilter(uint line, bool status);

  /** Set the bandwidth graph opacity setting. */
  int getBWGraphOpacity();
  /** Set the bandwidth graph opacity settings. */
  void setBWGraphOpacity(int value);

  /** Gets whether the bandwidth graph is always on top. */
  bool getBWGraphAlwaysOnTop();
  /** Sets whether the bandwidth graph is always on top. */
  void setBWGraphAlwaysOnTop(bool alwaysOnTop);
  




};

#endif

