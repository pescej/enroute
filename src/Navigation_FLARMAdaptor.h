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

#include <QTcpSocket>
#include <QTimer>
#include <QObject>

#include "AviationUnits.h"


namespace Navigation {

class FLARMAdaptor : public QObject {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit FLARMAdaptor(QObject *parent = nullptr);

    // Standard destructor
    ~FLARMAdaptor() override = default;

    /*! \brief Status codes */
    enum Status
    {
        OK, /*!< A FLARM device is connected and receives data. */
        WaitingForData, /*!< A FLARM device is connected, but no data has been received for more than 5 seconds.  */
        InOp /*!< No FLARM device connected */
    };
    Q_ENUM(Status)

    /*! \brief Pointer to static instance
     */
    static FLARMAdaptor *globalInstance();


    Q_PROPERTY(QString statusString READ statusString NOTIFY statusStringChanged)

    QString statusString() const
    {
        return _statusString;
    }

    Q_PROPERTY(QString FLARMHwVersion READ FLARMHwVersion NOTIFY FLARMHwVersionChanged)

    QString FLARMHwVersion() const
    {
        return _FLARMHwVersion;
    }

    Q_PROPERTY(QString FLARMSwVersion READ FLARMSwVersion NOTIFY FLARMSwVersionChanged)

    QString FLARMSwVersion() const
    {
        return _FLARMSwVersion;
    }

    Q_PROPERTY(QString FLARMObstVersion READ FLARMObstVersion NOTIFY FLARMObstVersionChanged)

    QString FLARMObstVersion() const
    {
        return _FLARMObstVersion;
    }

    Q_PROPERTY(QString FLARMSelfTest READ FLARMSelfTest NOTIFY FLARMSelfTestChanged)

    QString FLARMSelfTest() const
    {
        return _FLARMSelfTest;
    }

    /*! \brief Status of the FLARM adaptor

    This property holds the status of the FLARM adaptor class.
    */
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /*! \brief Getter function for the property with the same name

    @returns Property status
  */
    Status status() const
    {
        return _status;
    };

    void setSimulatorFile(QString fileName = QString() );

signals:
    void flarmSelfTestFailed();
    void FLARMSelfTestChanged();
    void FLARMHwVersionChanged();
    void FLARMSwVersionChanged();
    void FLARMObstVersionChanged();
    void receivingChanged();
    void statusStringChanged();
    void statusChanged();

private slots:
    void connectToFLARM();

    void receiveSocketConnected();
    void receiveSocketDisconnected();
    void receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError);

    void readFromStream();

    void readFromSimulatorStream();

    void processFLARMMessage(QString msg);

    void clearFlarmDeviceInfo();
    void setStatus();
    void setStatusString();

private:
    QTimer connectTimer;
    QTimer heartBeatTimer;

    QTcpSocket socket;
    QTextStream stream;

    // General status info. Should only been written to by setStatus()
    Status _status {InOp};

    // Detailed status info. Should only been written to by setStatusString()
    QString _statusString;

    // Height information
    AviationUnits::Distance GPGGAHeight;
    QDateTime GPGGATime;

    // FLARM information
    QString _FLARMHwVersion; // Hardware version
    QString _FLARMSwVersion; // Software version
    QString _FLARMObstVersion; // Name of obstacle database
    QString _FLARMSelfTest;

    // Simulator related members
    QFile simulatorFile;
    QTextStream simulatorStream;
    QTimer simulatorTimer;
    int lastTime {0};
    QString lastPayload;
};

}