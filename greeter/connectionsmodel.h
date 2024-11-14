/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CONNECTIONSMODEL_H
#define CONNECTIONSMODEL_H

#include <QAbstractListModel>

#include "connectionenum.h"

class ConnectionItem;
class ConnectionActivator;
class NMSecretAgent;

class ConnectionsModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ConnectionsModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void requestScanWifi() const;

    // processing a click on a connection element it two stages
    Q_INVOKABLE void onItemClick(const ConnectionItem &item)
    {
        actionDialog(item, selectActionForItem(item));
    }
    Q_INVOKABLE void onActionDialogComplete(QVariantMap data);
    Q_INVOKABLE void onActionDialogCancel(QVariantMap data);
    Q_INVOKABLE bool hasManagedWifiDevices();

    // for the button in the menubar
    Q_PROPERTY(QVariant primary READ getPrimary NOTIFY primaryChanged)

    Q_PROPERTY(bool networkingEnabled READ isNetworkingEnabled WRITE setNetworkingEnabled NOTIFY networkingEnabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ isWirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)

    Q_PROPERTY(bool allowSwitchWifi MEMBER m_allowSwitchWifi CONSTANT)
    Q_PROPERTY(bool allowSwitchNetworking MEMBER m_allowSwitchNetworking CONSTANT)
    Q_PROPERTY(bool allowNetworkControl MEMBER m_allowNetworkControl CONSTANT)
    Q_PROPERTY(bool allowModifyOwnSettings MEMBER m_allowModifyOwnSettings CONSTANT)

    Q_PROPERTY(bool networkManagerAvailable MEMBER m_networkManagerAvailable CONSTANT)

public Q_SLOTS:
    void getSecrets(const QString &connectionName, const QList<QVariantMap> &secrets);
    void gotSecrets(const QVariantMap &data);

Q_SIGNALS:
    void primaryChanged();
    void networkingEnabledChanged();
    void wirelessEnabledChanged();
    void showDialog(const QVariantMap &data);

private:

    class Util;

    void fillConnections();

    void checkItem(ConnectionItem &&item);

    // change the container elements and notify the model correctly
    void insertItem(int index, ConnectionItem &&item);
    void moveItem(int indexFrom, int indexTo);
    void updateItem(int index, ConnectionItem &&item);
    void removeItem(int index);

    void setNetworkingEnabled(bool value);
    void setWirelessEnabled(bool value);
    bool isNetworkingEnabled();
    bool isWirelessEnabled();
    void updatePrimaryConnection();
    QVariant getPrimary();

    void connectItem(const ConnectionItem &item);
    void disconnectItem(const ConnectionItem &item);
    void createAndConnect(QVariantMap data);
    void deleteConnection(const ConnectionItem &item);
    void abortConnection(const ConnectionItem &item);

    void actionDialog(const ConnectionItem &item, ConnectionEnum::Action action);
    void errorPopup(const QString &connectionName, const QString &message);

    ConnectionEnum::Action selectActionForItem(const ConnectionItem &item);

    static bool precede(const ConnectionItem &i1, const ConnectionItem &i2);
    // note that this is not a complete equality of the elements
    static bool sameConnection(const ConnectionItem &i1, const ConnectionItem &i2);

    QVector<ConnectionItem> m_connections;
    QScopedPointer<ConnectionItem> m_primary;
    QScopedPointer<NMSecretAgent> m_secretAgent;
    QString m_username;
    ConnectionActivator *m_activator;

    bool m_allowSwitchWifi;
    bool m_allowSwitchNetworking;
    bool m_allowNetworkControl;
    bool m_allowModifyOwnSettings;
    bool m_networkManagerAvailable;
};

#endif // CONNECTIONSMODEL_H
