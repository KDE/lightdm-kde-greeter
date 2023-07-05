/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.ru>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QDebug>
#include <QLoggingCategory>
#include <QDBusConnection>

#include <NetworkManagerQt/Manager>

#include <pwd.h>

#include "wifikeeper.h"

WifiKeeperApp::WifiKeeperApp(int &argc, char **argv)
    : QCoreApplication(argc, argv)
{
    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning() << "cannot connect to the D-Bus session bus";
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerService(QStringLiteral("org.altlinux.LightDMKdeGreeter.WifiKeeper"))) {
        qWarning() << "failed to register service:" << QDBusConnection::sessionBus().lastError().message();
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerObject(QStringLiteral("/"), this, QDBusConnection::ExportAllSlots)) {
        qWarning() << "some problems during object registration";
        exit(1);
    }
    qInfo() << "registered with dbus";
    qInfo() << "NetworkManager version:" << NetworkManager::version();
}

int main(int argc, char **argv)
{
    // qt networkmanager wrapper is very verbose
    QLoggingCategory::setFilterRules(QStringLiteral("kf.networkmanagerqt=false"));
    WifiKeeperApp app(argc, argv);
    return app.exec();
}

QString WifiKeeperApp::addAndActivateConnection(NMVariantMapMap connection, QDBusObjectPath device, QDBusObjectPath specificObject, QVariantMap options)
{
    auto reply = NetworkManager::addAndActivateConnection2(connection, device.path(), specificObject.path(), options);
    reply.waitForFinished();
    if (reply.isError()) return {};
    return reply.argumentAt(0).value<QDBusObjectPath>().path();
}
