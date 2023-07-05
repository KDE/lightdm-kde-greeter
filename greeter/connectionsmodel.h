/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

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

#ifndef CONNECTIONSMODEL_H
#define CONNECTIONSMODEL_H

#include <QAbstractListModel>

#include "connectionenum.h"

class ConnectionItem;
class ConnectionActivator;

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
        emit showDialog(item, selectActionForItem(item));
    }
    Q_INVOKABLE void onActionDialogComplete(QVariantMap data);

    // for the button in the menubar
    Q_PROPERTY(QVariant primary READ getPrimary NOTIFY primaryChanged)

    Q_PROPERTY(bool networkingEnabled READ isNetworkingEnabled WRITE setNetworkingEnabled NOTIFY networkingEnabledChanged)
    Q_PROPERTY(bool wirelessEnabled READ isWirelessEnabled WRITE setWirelessEnabled NOTIFY wirelessEnabledChanged)

Q_SIGNALS:
    void primaryChanged();
    void networkingEnabledChanged();
    void wirelessEnabledChanged();
    void showDialog(const ConnectionItem &item, ConnectionEnum::Action action);

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

    ConnectionEnum::Action selectActionForItem(const ConnectionItem &item);

    static bool precede(const ConnectionItem &i1, const ConnectionItem &i2);
    // note that this is not a complete equality of the elements
    static bool sameConnection(const ConnectionItem &i1, const ConnectionItem &i2);

    QVector<ConnectionItem> m_connections;
    QScopedPointer<ConnectionItem> m_primary;
    QString m_username;
    ConnectionActivator *m_activator;
};

#endif // CONNECTIONSMODEL_H
