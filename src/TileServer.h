/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#pragma once

#include <qhttpengine/filesystemhandler.h>
#include <qhttpengine/server.h>

#include <QPointer>


/*! \brief HTTP server for mapbox' MBTiles files
  
  This class features an HTTP server that is able to serve MBTiles, for use in
  Mapbox-powered geographical map applications. The server is build on
  nitroshare's excellent qhttpengine code
  (https://github.com/nitroshare/qhttpengine). This class is in factt a thin
  wrapper around QHttpEngine::Server, with only a few convenience methods
  added. It inherits thus indirectly from QTcpServer, so that the documentation
  for this class applies.
  
  This server is able to handle several sets of MBTiles files. Each set is
  available under a single URL, and the MBTile files in each set are expected to
  contain identical metadata. To serve a tile request, the server goes through
  the MBTiles files in random order and serves the first tile it can find. Tiles
  contained in more than one file need thus to be identical. A typical use case
  would be a server that contains to sets of files, one set with vector tiles
  containing openstreetmap data and one set with raster data used for
  hillshading. Each set contains two MBTiles files, one for Africa and one for
  Europe.
*/

class TileServer : public QHttpEngine::Server
{
  Q_OBJECT
  
public:
  /*! \brief Create a new tile server
    
    This constructor sets up a new tile server. The server will serve the static
    content found in the Qt ressource system under ":/static", as well as sets
    of tiles added using the method addMbtilesFileSet().
     
    @param baseUrl The URL under which the tile will be available.  If this URL
    is empty, then the TileJSON generated by the server will direct clients to
    retrieve tiles from serverUrl(); this is often a string of the form
    "http://localhost:8080". This is not the correct behaviour in all
    settings. For instance, if the server is visible to the outside world
    through an reverse ssl-proxy under the Url "https://myserver.com" then you
    should pass that URL here.
    
    @param parent The standard QObject parent
  */
  explicit TileServer(QUrl baseUrl=QUrl(), QObject *parent = nullptr);
  
  // Standard destructor
  ~TileServer() override = default;
  
  /*! \brief URL under which this server is presently reachable
    
    If the server is listening to incoming connections, this is a URL of the
    form "http://serverAddress:serverPort", typically something like
    "http://127.0.0.1:3470". If the server is not listening to incoming
    connections, an empty string is returned.

    @returns URL under which this server is presently reachable
  */
  QString serverUrl() const;
			   
public slots:
  /*! \brief Add a new set of tile files
    
    This method adds a new set of tile files, that will be available under
    serverUrl()+"/baseName" (typically, this is a URL of the form
    'http://localhost:8080/basename').
   
    @param baseMapsWithFiles The name of one or more mbtile files on the disk,
    which are expected to conform to the MBTiles Specification 1.3
    (https://github.com/mapbox/mbtiles-spec/blob/master/1.3/spec.md). These
    files must exist until the file set is removed or the sever is destructed,
    or else replies to tile requests will yield undefined results. The tile
    files are expected to agree in their metadata, and the metadata
    (attribution, description, format, name, minzoom, maxzoom) is read only from
    one of the files (a random one, in fact). If a tile is contained in more
    than one of the files, the data is expected to be identical in each of the
    files.
     
    @param baseName The path under which the tiles willconst be available.
  */
  void addMbtilesFileSet(const QList<QPointer<Downloadable>>& baseMapsWithFiles, const QString& baseName);

  /*! \brief Removes a set of tile files
   
    @param path Path of tiles to remove
   */
  void removeMbtilesFileSet(const QString& path);
  
private:
  Q_DISABLE_COPY_MOVE(TileServer)

  void setUpTileHandlers();
  
  QPointer<QHttpEngine::FilesystemHandler> currentFileSystemHandler;
  
  QMap<QString,QList<QPointer<Downloadable>>> mbtileFileNameSets;
  
  QUrl _baseUrl;
};
