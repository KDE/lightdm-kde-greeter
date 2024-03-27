/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.ru>

SPDX-License-Identifier: GPL-3.0-or-later
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
