/****************************************************************
 *  RetroShare is distributed under the following license:
 *
 *  Copyright (C) 2009 The RetroShare Team, Oleksiy Bilyanskyy
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

#include "IMHistoryWriter.h"

#include <QFile>

#include <QDebug>

#include <QDateTime>

//=============================================================================

IMHistoryWriter::IMHistoryWriter()
                :errMess("No error")
{
  // nothing to do here
}

//=============================================================================

bool
IMHistoryWriter::write(QList<IMHistoryItem>& itemList,
                            const QString fileName  )
{
    qDebug() << "  IMHistoryWriter::write is here" ;

    errMess = "No error";

//==== check for file and open it
    QFile fl(fileName);
    if (fl.open(QIODevice::WriteOnly | QIODevice::Truncate));
    else
    {
        errMess = QString("error opening file %1 (code %2)")
                         .arg(fileName).arg( fl.error() );
        return false ;
    }

    //==== set the file, and check it once more
    setDevice(&fl);

    writeStartDocument();
    writeDTD("<!DOCTYPE history_file>");
    writeStartElement("history_file");
    writeAttribute("format_version", "1.0");

    foreach(IMHistoryItem item, itemList)
    {
        writeStartElement("message");
        writeAttribute( "dt", QString::number(item.time().toTime_t()) ) ;
        writeAttribute( "sender", item.sender() );
        writeAttribute( "receiver", item.receiver() ) ;
        writeCharacters(  item.text()); 
        writeEndElement();
    }

    writeEndDocument() ;

    fl.close();

    qDebug() << "  IMHistoryWriter::write done" ;

    return true;
}