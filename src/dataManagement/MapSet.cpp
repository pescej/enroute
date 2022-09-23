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

#include "MapSet.h"


DataManagement::MapSet::MapSet(QVector<DataManagement::Downloadable*> maps, QObject* parent)
    : QObject(parent)
{
    foreach(auto map, maps)
    {
        m_maps.append(map);
    }
    m_maps.removeAll(nullptr);

#warning signals are emitted too often
    foreach(auto map, m_maps)
    {
        setObjectName(map->objectName());
        m_section = map->section();

        connect(map, &DataManagement::Downloadable::error, this, &DataManagement::MapSet::error);
        connect(map, &DataManagement::Downloadable::downloadingChanged, this, &DataManagement::MapSet::downloadingChanged);
        connect(map, &DataManagement::Downloadable::fileContentChanged, this, &DataManagement::MapSet::descriptionChanged);
        connect(map, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::hasFileChanged);
        connect(map, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::updatableChanged);
        connect(map, &DataManagement::Downloadable::infoTextChanged, this, &DataManagement::MapSet::infoTextChanged);
        connect(map, &DataManagement::Downloadable::updatableChanged, this, &DataManagement::MapSet::updatableChanged);
    }
}


auto DataManagement::MapSet::description() -> QString
{
    QString result;

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        switch(map->contentType())
        {
        case Downloadable::AviationMap:
            result += "<h4>"+tr("Aviation Map")+"</h4>";
            break;
        case Downloadable::BaseMap:
            result += "<h4>"+tr("Base Map")+"</h4>";
            break;
        case Downloadable::TerrainMap:
            result += "<h4>"+tr("Terrain Map")+"</h4>";
            break;
        case Downloadable::Data:
            result += "<h4>"+tr("Data")+"</h4>";
            break;
        }
        result += map->description();
    }
    return result;
}


auto DataManagement::MapSet::downloading() -> bool
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->downloading())
        {
            return true;
        }
    }
    return false;
}


auto DataManagement::MapSet::hasFile() -> bool
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->hasFile())
        {
            return true;
        }
    }
    return false;
}


auto DataManagement::MapSet::infoText() -> QString
{
    QString result;

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (!result.isEmpty())
        {
            result += QLatin1String("<br>");
        }
        switch(map->contentType())
        {
        case Downloadable::AviationMap:
            result += QStringLiteral("%1: %2").arg(tr("Aviation Map"), map->infoText());
            break;
        case Downloadable::BaseMap:
            result += QStringLiteral("%1: %2").arg(tr("Base Map"), map->infoText());
            break;
        case Downloadable::TerrainMap:
            result += QStringLiteral("%1: %2").arg(tr("Terrain Map"), map->infoText());
            break;
        case Downloadable::Data:
            result += QStringLiteral("%1: %2").arg(tr("Data"), map->infoText());
            break;
        }
    }
    return result;
}


auto DataManagement::MapSet::updatable() -> bool
{
    if (!hasFile())
    {
        return false;
    }

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->updatable() || !map->hasFile())
        {
            return true;
        }
    }
    return false;
}


void DataManagement::MapSet::deleteFile()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->deleteFile();
    }
}


void DataManagement::MapSet::startFileDownload()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->startFileDownload();
    }
}


void DataManagement::MapSet::stopFileDownload()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->stopFileDownload();
    }
}


void DataManagement::MapSet::update()
{
    if (!updatable())
    {
        return;
    }

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->updatable() || !map->hasFile())
        {
            map->startFileDownload();
        }
    }
}
