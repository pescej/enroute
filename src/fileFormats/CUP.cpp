/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QFile>

#include "fileFormats/CUP.h"

#include "fileFormats/DataFileAbstract.h"
#include <cmath>

//
// Private helper functions
//

QStringList FileFormats::CUP::parseCSV(const QString& string)
{
    // Thanks to https://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt

    enum State
    {
        Normal,
        Quote
    } state = Normal;
    QStringList fields;
    fields.reserve(10);
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        const QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                state = Quote;
                value += current;
            }

            // Other character
            else
            {
                value += current;
            }
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i < string.size())
                {
                    // A double double-quote?
                    if (i + 1 < string.size() && string.at(i + 1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        state = Normal;
                        value += '"';
                    }
                }
            }

            // Other character
            else
            {
                value += current;
            }
        }
    }

    if (!value.isEmpty())
    {
        fields.append(value.trimmed());
    }

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The outermost quotes are removed here.
    for (auto &field : fields)
    {
        if (field.length() >= 1 && field.at(0) == '"')
        {
            field = field.mid(1);
            if (field.length() >= 1 && field.right(1) == '"')
            {
                field = field.left(field.length() - 1);
            }
        }
    }

    return fields;
}

GeoMaps::Waypoint FileFormats::CUP::readWaypoint(const QString &line)
{
    auto fields = parseCSV(line);
    if (fields.size() < 6)
    {
        return {};
    }

    // Get Name
    auto name = fields[0];

    // Get Latitude
    double lat = NAN;
    {
        auto latString = fields[3];
        if (latString.size() != 9)
        {
            return {};
        }
        if ((latString[8] != 'N') && (latString[8] != 'S'))
        {
            return {};
        }
        bool ok = false;
        lat = latString.left(2).toDouble(&ok);
        if (!ok)
        {
            return {};
        }
        lat = lat + latString.mid(2, 6).toDouble(&ok) / 60.0;
        if (!ok)
        {
            return {};
        }
        if (latString[8] == 'S')
        {
            lat = -lat;
        }
    }

    // Get Longitude
    double lon = NAN;
    {
        auto longString = fields[4];
        if (longString.size() != 10)
        {
            return {};
        }
        if ((longString[9] != 'W') && (longString[9] != 'E'))
        {
            return {};
        }
        bool ok = false;
        lon = longString.left(3).toDouble(&ok);
        if (!ok)
        {
            return {};
        }
        lon = lon + longString.mid(3, 6).toDouble(&ok) / 60.0;
        if (!ok)
        {
            return {};
        }
        if (longString[9] == 'W')
        {
            lon = -lon;
        }
    }

    double ele = NAN;
    {
        auto eleString = fields[5];
        bool ok = false;
        if (eleString.endsWith(u"m"))
        {
            ele = eleString.chopped(1).toDouble(&ok);
        }
        if (eleString.endsWith(u"ft"))
        {
            ele = eleString.chopped(1).toDouble(&ok) * 0.3048;
        }
        if (!ok)
        {
            return {};
        }
    }

    // Get additional information
    QStringList notes;
    if (fields.size() >= 8)
    {
        notes += QObject::tr("Direction: %1°", "GeoMaps::CUP").arg(fields[7]);
    }
    if (fields.size() >= 9)
    {
        notes += QObject::tr("Length: %1", "GeoMaps::CUP").arg(fields[8]);
    }
    if ((fields.size() >= 11) && (!fields[10].isEmpty()))
    {
        notes += QObject::tr("Frequency: %1", "GeoMaps::CUP").arg(fields[10]);
    }
    if (fields.size() >= 12)
    {
        notes += fields[11];
    }

    GeoMaps::Waypoint result(QGeoCoordinate(lat, lon, ele));
    result.setName(name);
    if (!notes.isEmpty())
    {
        result.setNotes(notes.join(" • "));
    }
    return result;
}


FileFormats::CUP::CUP(const QString& fileName)
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    auto success = file->open(QIODevice::ReadOnly);
    if (!success)
    {
        setError(QObject::tr("Cannot open CUP file %1 for reading.", "FileFormats::CUP").arg(fileName));
        return;
    }

    QTextStream stream(file.data());
    QString line;
    stream.readLineInto(&line);
    int lineNumber = 0;
    while (stream.readLineInto(&line))
    {
        lineNumber++;
        if (line.contains(u"-----Related Tasks-----"))
        {
            break;
        }
        auto waypoint = readWaypoint(line);
        if (!waypoint.isValid())
        {
            setError(QObject::tr("Error reading line %1 in the CUP file %1.", "FileFormats::CUP").arg(lineNumber).arg(fileName));
            return;
        }
        m_waypoints << waypoint;
    }
}
