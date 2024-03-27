/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WIFIKEEPER_H
#define WIFIKEEPER_H

#include <QCoreApplication>
#include <NetworkManagerQt/GenericTypes>

class WifiKeeperApp: public QCoreApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.altlinux.LightDMKdeGreeter.WifiKeeper.interface")

public Q_SLOTS:
    QString addAndActivateConnection(NMVariantMapMap connection, QDBusObjectPath device, QDBusObjectPath specificObject, QVariantMap options);

public:
    WifiKeeperApp(int &argc, char **argv);
};

#endif /* WIFIKEEPER_H */
