/****************************************************************
 *  Retroshare is distributed under the following license:
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



#include <QDir>
#include <QCoreApplication>
#include <lang/languagesupport.h>

#include <rshare.h>

#include "rsharesettings.h"

#if defined(Q_WS_WIN)
#include <util/registry.h>
#include <util/win32.h>
#endif


/* Rshare's Settings */
#define SETTING_LANGUAGE            "LanguageCode"
#define SETTING_STYLE               "InterfaceStyle"
#define SETTING_DATA_DIRECTORY      "DataDirectory"

#define SETTING_BWGRAPH_FILTER        "StatisticDialog/BWLineFilter"
#define SETTING_BWGRAPH_OPACITY       "StatisticDialog/Opacity"
#define SETTING_BWGRAPH_ALWAYS_ON_TOP "StatisticDialog/AlwaysOnTop"

/* Default Rshare Settings */
#if defined(Q_WS_MAC)
#define DEFAULT_STYLE               "macintosh (aqua)"
#else
#define DEFAULT_STYLE               "plastique"
#endif

#define DEFAULT_LANGUAGE            LanguageSupport::defaultLanguageCode()

#define DEFAULT_OPACITY             100


#if defined(Q_OS_WIN32)

#define STARTUP_REG_KEY        "Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define Rshare_REG_KEY        "RetroShare" 
#else

#endif

/* Default bandwidth graph settings */
#define DEFAULT_BWGRAPH_FILTER          (BWGRAPH_SEND|BWGRAPH_REC)
#define DEFAULT_BWGRAPH_ALWAYS_ON_TOP   false

/** The location of Rshare's settings and configuration file. */
#define SETTINGS_FILE   (Rshare::dataDirectory() + "/RetroShare.conf")


/** Default Constructor */
RshareSettings::RshareSettings()
: QSettings(SETTINGS_FILE, QSettings::IniFormat)
{  
}

/** Resets all of Rshare's settings. */
void
RshareSettings::reset()
{
  QSettings settings(SETTINGS_FILE, QSettings::IniFormat);
  settings.clear();
}

/** Gets the currently preferred language code for Rshare. */
QString
RshareSettings::getLanguageCode()
{
  return value(SETTING_LANGUAGE, DEFAULT_LANGUAGE).toString();
}

/** Sets the preferred language code. */
void
RshareSettings::setLanguageCode(QString languageCode)
{
  setValue(SETTING_LANGUAGE, languageCode);
}

/** Gets the interface style key (e.g., "windows", "motif", etc.) */
QString
RshareSettings::getInterfaceStyle()
{
  return value(SETTING_STYLE, DEFAULT_STYLE).toString();
}

/** Sets the interface style key. */
void
RshareSettings::setInterfaceStyle(QString styleKey)
{
  setValue(SETTING_STYLE, styleKey);
}


/** Returns the bandwidth line filter. */
uint
RshareSettings::getBWGraphFilter()
{
  return value(SETTING_BWGRAPH_FILTER, DEFAULT_BWGRAPH_FILTER).toUInt(); 
}

/** Saves the setting for whether or not the given line will be graphed */
void
RshareSettings::setBWGraphFilter(uint line, bool status)
{
  uint filter = getBWGraphFilter();
  filter = (status ? (filter | line) : (filter & ~(line)));
  setValue(SETTING_BWGRAPH_FILTER, filter);
}

/** Get the level of opacity for the BandwidthGraph window. */
int
RshareSettings::getBWGraphOpacity()
{
  return value(SETTING_BWGRAPH_OPACITY, DEFAULT_OPACITY).toInt();
}

/** Set the level of opacity for the BandwidthGraph window. */
void
RshareSettings::setBWGraphOpacity(int value)
{
  setValue(SETTING_BWGRAPH_OPACITY, value);
}

/** Gets whether the bandwidth graph is always on top when displayed. */
bool
RshareSettings::getBWGraphAlwaysOnTop()
{
  return value(SETTING_BWGRAPH_ALWAYS_ON_TOP,
               DEFAULT_BWGRAPH_ALWAYS_ON_TOP).toBool();
}

/** Sets whether the bandwidth graph is always on top when displayed. */
void
RshareSettings::setBWGraphAlwaysOnTop(bool alwaysOnTop)
{
  setValue(SETTING_BWGRAPH_ALWAYS_ON_TOP, alwaysOnTop);
}











