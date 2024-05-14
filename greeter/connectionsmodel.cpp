/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QTimer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#define TRANSLATION_DOMAIN "lightdm_kde_greeter"
#include <KLocalizedString>

#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>
#include <NetworkManagerQt/WirelessSetting>
#include <NetworkManagerQt/Utils>

#include <cassert>
#include <pwd.h>

#include "connectionsmodel.h"
#include "connectionitem.h"
#include "connectionactivator.h"

class ConnectionsModel::Util
{
public:
    static NetworkManager::Connection::Ptr findConnectionInDevice(const QString &connectionPath, NetworkManager::Device::Ptr dev)
    {
        if (dev->activeConnection() && dev->activeConnection()->path() == connectionPath) return dev->activeConnection()->connection();
        for (auto &conn : dev->availableConnections()) {
            if (conn->path() == connectionPath) return conn;
        }
        return {};
    }

    static NetworkManager::Device::Ptr findDeviceForConnectionPath(const QString &connectionPath)
    {
        for (auto &dev : NetworkManager::networkInterfaces()) {
            if (findConnectionInDevice(connectionPath, dev)) return dev;
        }
        return {};
    }

    static NetworkManager::WirelessDevice::Ptr findDeviceForAccessPointPath(const QString &accessPointPath)
    {
        for (auto &dev : NetworkManager::networkInterfaces()) {
            if (dev->type() != NetworkManager::Device::Wifi) continue;
            auto wifiDev = dev.objectCast<NetworkManager::WirelessDevice>();
            auto ap = wifiDev->findAccessPoint(accessPointPath);
            if (ap) return wifiDev;
        }
        return {};
    }

    static void disconnectAllDeviceEvents()
    {
        for (auto &dev : NetworkManager::networkInterfaces()) {
            disconnect(dev.data(), &NetworkManager::Device::stateChanged, nullptr, nullptr);
        }
    }

    static bool isConnectionCreated(const ConnectionItem &item)
    {
        return (item.path.length() != 0 && !item.path.startsWith(QStringLiteral("/org/freedesktop/NetworkManager/AccessPoint")));
    }

    template <class Reply, class OnErrorCb, class OnSuccessCb>
    static void handleDBusCall(ConnectionsModel *cm, Reply reply, OnErrorCb onError, OnSuccessCb onSuccess)
    {
        auto pendingCallWatcher = new QDBusPendingCallWatcher(reply, cm);
        connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, cm, [onError, onSuccess] (QDBusPendingCallWatcher *w) {
            w->deleteLater();
            // need to specify a specific type here (not auto) because it
            // depends on what signature we expect in the DBus response
            Reply reply = *w;
            if (reply.isError()) {
                onError();
            } else {
                onSuccess();
            }
        });
    }

    template <class Reply>
    static void handleDBusCall(ConnectionsModel *cm, Reply reply)
    {
        handleDBusCall(cm, reply, [reply]{ qWarning() << "handleDBusCall error: " << reply.error(); }, []{});
    }

    template <class OnErrorCb>
    static void waitForDevice(ConnectionsModel *cm, NetworkManager::Device *dev, OnErrorCb onError)
    {
        disconnectAllDeviceEvents();
        connect(dev, &NetworkManager::Device::stateChanged,
                cm, [onError] (NetworkManager::Device::State newstate,
                               NetworkManager::Device::State /* oldstate */,
                               NetworkManager::Device::StateChangeReason reason) {
                    if (newstate == NetworkManager::Device::Failed) {
                        qInfo() << "waitForDevice: new device state is \"failed\", reason:" << reason;
                        onError();
                    } else if (newstate == NetworkManager::Device::Activated) {
                        disconnectAllDeviceEvents();
                    }
                });
    }

    template <class Reply, class OnErrorCb>
    static void handleDBusDeviceError(ConnectionsModel *cm, NetworkManager::Device *dev, Reply reply, OnErrorCb onError)
    {
        handleDBusCall(cm, reply, onError, [cm, dev, onError] { waitForDevice(cm, dev, onError); });
    }

};

ConnectionsModel::ConnectionsModel(QObject *parent) :
    QAbstractListModel(parent),
    m_primary(new ConnectionItem()),
    m_activator(new ConnectionActivator(this))
{
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::networkingEnabledChanged, this, &ConnectionsModel::networkingEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::wirelessEnabledChanged, this, &ConnectionsModel::wirelessEnabledChanged);
    connect(NetworkManager::notifier(), &NetworkManager::Notifier::primaryConnectionChanged, this, &ConnectionsModel::updatePrimaryConnection);

    auto pw = getpwuid(getuid());
    if (!pw || strlen(pw->pw_name) == 0) {
        qWarning("%s: unable to get user data", __FUNCTION__);
    } else {
        m_username = QLatin1String(pw->pw_name);
    }

    requestScanWifi();
    fillConnections();

    QTimer *checkConnectionsTimer = new QTimer(this);
    checkConnectionsTimer->setInterval(1000);
    connect(checkConnectionsTimer, &QTimer::timeout, this, &ConnectionsModel::fillConnections);
    checkConnectionsTimer->start();

    updatePrimaryConnection();
}

void ConnectionsModel::fillConnections()
{
    NetworkManager::Connection::List allConnections = NetworkManager::listConnections();

    // will mark connections found in this iteration to remove all unmarked
    for (auto &c : m_connections) {
        c.mark = false;
    }

    // wired networks
    for (const auto &c : allConnections) {

        if (!c->isValid()) continue;

        QString path = c->path();
        if ((c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wired) && !c->uuid().isEmpty()) {
            ConnectionEnum::State state = ConnectionEnum::STATE_OFF;
            for (const auto &ac : NetworkManager::activeConnections()) {
                if (ac->uuid() == c->uuid()) {
                    path = ac->path();
                    state = ac->state() == ac->Activated ? ConnectionEnum::STATE_ON : ConnectionEnum::STATE_WAIT;
                    break;
                }
            }

            checkItem(std::move(ConnectionItem{}
                                .setType(ConnectionEnum::TYPE_WIRED)
                                .setName(c->name())
                                .setPath(path)
                                .setState(state)));
        }
    }

    // show available wifi networks
    if (NetworkManager::isWirelessEnabled()) {
        for (const auto &device : NetworkManager::networkInterfaces()) {
            if (device->type() != NetworkManager::Device::Wifi) continue;

            auto wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();

            for (const auto &network : wifiDevice->networks()) {
                auto accessPoint = network->referenceAccessPoint();

                if (!accessPoint) continue;

                // check if we have a connection for this SSID
                NetworkManager::Connection::Ptr conn;
                for (const NetworkManager::Connection::Ptr &c : allConnections) {
                    if (c->isValid() && (c->settings()->connectionType() == NetworkManager::ConnectionSettings::Wireless)) {
                        auto setting = c->settings()->setting(NetworkManager::Setting::Wireless);
                        auto s = setting.staticCast<NetworkManager::WirelessSetting>();

                        if (s->ssid() == network->ssid().toLocal8Bit()) {
                            conn = c;
                            break;
                        }
                    }
                }

                QString entryText = conn ? conn->name() : network->ssid();
                QString path = accessPoint->uni();

                int flags = 0;

                if (accessPoint->wpaFlags() || accessPoint->rsnFlags()) {
                    flags |= ConnectionEnum::FLAG_LOCKED;
                }

                ConnectionEnum::State entryState = ConnectionEnum::STATE_OFF;
                if (conn) {
                    path = conn->path();
                    auto permissions = conn->settings()->permissions();
                    if (permissions.contains(m_username)) {
                        flags |= ConnectionEnum::FLAG_PRIVATE;
                    }
                    for (const auto &ac : NetworkManager::activeConnections()) {
                        if (ac->uuid() == conn->uuid()) {
                            entryState = ac->state() == ac->Activated ? ConnectionEnum::STATE_ON : ConnectionEnum::STATE_WAIT;
                            path = ac->path();
                            break;
                        }
                    }
                }

                checkItem(std::move(ConnectionItem{}
                                    .setType(ConnectionEnum::TYPE_WIRELESS)
                                    .setFlags(flags)
                                    .setName(entryText)
                                    .setPath(path)
                                    .setSignalStrength(network->signalStrength())
                                    .setWpaFlags(accessPoint->rsnFlags() ? accessPoint->rsnFlags() : accessPoint->wpaFlags())
                                    .setState(entryState)));
            }
        }
    }

    int i = 0;
    while (i < m_connections.size()) {
        if (!m_connections[i].mark) removeItem(i);
        else ++i;
    }

    // update signal level for menubar button
    for (auto &item : m_connections) {
        if (sameConnection(item, *m_primary)) {
            if (m_primary->signalStrength == item.signalStrength && m_primary->flags == item.flags) break;
            m_primary->setSignalStrength(item.signalStrength).setFlags(item.flags);
            emit primaryChanged();
            break;
        }
    }
}

void ConnectionsModel::insertItem(int index, ConnectionItem &&item)
{
    assert(index <= m_connections.size() && index >= 0);
    beginInsertRows(QModelIndex(), index, index);
    m_connections.insert(index, item);
    endInsertRows();
}

void ConnectionsModel::removeItem(int index)
{
    assert(index < m_connections.size() && index >= 0);
    beginRemoveRows(QModelIndex(), index, index);
    m_connections.remove(index);
    endRemoveRows();
}

void ConnectionsModel::updateItem(int index, ConnectionItem &&item)
{
    assert(index < m_connections.size() && index >= 0);
    m_connections[index] = std::move(item);
    QModelIndex qIndex = createIndex(index, 0);
    emit dataChanged(qIndex, qIndex);
}

void ConnectionsModel::moveItem(int indexFrom, int indexTo)
{
    assert(indexFrom < m_connections.size() && indexFrom >= 0);
    assert(indexTo <= m_connections.size() && indexFrom >= 0);
    // https://doc.qt.io/qt-6/qabstractitemmodel.html#beginMoveRows
    if (indexFrom == indexTo || indexFrom == indexTo - 1) {
        qWarning("%s: noop move (%d -> %d)", __FUNCTION__, indexFrom, indexTo);
        return;
    }
    beginMoveRows(QModelIndex(), indexFrom, indexFrom, QModelIndex(), indexTo);
    // QVector moves elements differently than QAbstractItemModel
    if (indexTo > indexFrom) --indexTo;
    m_connections.move(indexFrom, indexTo);
    endMoveRows();
}

void ConnectionsModel::checkItem(ConnectionItem &&item)
{
    item.mark = true;

    int oldIndex;
    for (oldIndex = 0; oldIndex < m_connections.size(); ++oldIndex) {
        if (sameConnection(m_connections[oldIndex], item)) break;
    }

    // new connection
    if (oldIndex == m_connections.size()) {
        int i;
        for (i = 0; i < m_connections.size(); ++i) {
            if (precede(item, m_connections[i])) break;
        }
        insertItem(i, std::move(item));
        return;
    }

    // nothing changed
    if (m_connections[oldIndex] == item) {
        m_connections[oldIndex].mark = true;
        return;
    }

    int newIndex = 0;
    for (newIndex = 0; newIndex < m_connections.size(); ++newIndex) {
        if (precede(item, m_connections[newIndex])) break;
    }

    // changed, but remained at the same index
    if (newIndex == oldIndex || newIndex == oldIndex + 1) {
        updateItem(oldIndex, std::move(item));
        return;
    }

    // the only option left is to move the element
    moveItem(oldIndex, newIndex);
    if (newIndex > oldIndex) --newIndex;
    updateItem(newIndex, std::move(item));
}

bool ConnectionsModel::precede(const ConnectionItem &i1, const ConnectionItem &i2)
{
    if (i1.type != i2.type) return i1.type < i2.type;
    if (i1.state != i2.state) return i1.state > i2.state;
    bool locked1 = i1.flags & ConnectionEnum::FLAG_LOCKED;
    bool locked2 = i2.flags & ConnectionEnum::FLAG_LOCKED;
    if (locked1 != locked2) return locked1 > locked2;
    return i1.name < i2.name;
}

bool ConnectionsModel::sameConnection(const ConnectionItem &i1, const ConnectionItem &i2)
{
    return i1.type == i2.type && i1.name == i2.name;
}

int ConnectionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_connections.size();
}

QVariant ConnectionsModel::data(const QModelIndex &index, int role) const
{
    int i = index.row();
    if (role == Qt::UserRole) return QVariant::fromValue(m_connections[i]);
    return QVariant();
}

QHash<int,QByteArray> ConnectionsModel::roleNames() const
{
    return {{ Qt::UserRole, "item" }};
}

void ConnectionsModel::requestScanWifi() const
{
    if (!NetworkManager::isWirelessEnabled()) return;
    for (const NetworkManager::Device::Ptr &device : NetworkManager::networkInterfaces()){
        if ( device->type() != NetworkManager::Device::Wifi ) continue;
        NetworkManager::WirelessDevice::Ptr wifiDevice = device.objectCast<NetworkManager::WirelessDevice>();
        wifiDevice->requestScan();
    }
}

bool ConnectionsModel::isNetworkingEnabled()
{
    return NetworkManager::isNetworkingEnabled();
}

bool ConnectionsModel::isWirelessEnabled()
{
    return NetworkManager::isWirelessEnabled();
}

void ConnectionsModel::setNetworkingEnabled(bool value)
{
    if (isNetworkingEnabled() == value) return;
    NetworkManager::setNetworkingEnabled(value);
}

void ConnectionsModel::setWirelessEnabled(bool value)
{
    if (isWirelessEnabled() == value) return;
    NetworkManager::setWirelessEnabled(value);
}

void ConnectionsModel::connectItem(const ConnectionItem &item)
{
    if (item.state != ConnectionEnum::STATE_OFF) return;
    if (!Util::isConnectionCreated(item)) {
        qWarning("%s: attempting to connect without creating to a non-existing connection", __FUNCTION__);
        return;
    }
    auto dev = Util::findDeviceForConnectionPath(item.path);
    if (!dev) {
        qWarning("%s: could not find device for connection: %s", __FUNCTION__, qPrintable(item.path));
        return;
    }
    Util::handleDBusDeviceError(this, dev.data(), NetworkManager::activateConnection(item.path, dev->uni(), {}),
                                [this, item] {
                                    emit showDialog(item, ConnectionEnum::ACTION_FAILED_TO_CONNECT);
                                });
}

void ConnectionsModel::disconnectItem(const ConnectionItem &item)
{
    if (item.state != ConnectionEnum::STATE_ON) return;
    Util::handleDBusCall(this, NetworkManager::deactivateConnection(item.path));
}

void ConnectionsModel::updatePrimaryConnection()
{
    NetworkManager::ActiveConnection::Ptr conn(NetworkManager::primaryConnection());
    if (!conn || !conn->connection()) {
        m_primary->setName(ki18n("No connection").toString()).setType(ConnectionEnum::TYPE_NONE);
    } else {
        auto c = conn->connection();
        ConnectionEnum::Type type;
        switch (c->settings()->connectionType()) {
            case NetworkManager::ConnectionSettings::Wired: type = ConnectionEnum::TYPE_WIRED; break;
            case NetworkManager::ConnectionSettings::Wireless: type = ConnectionEnum::TYPE_WIRELESS; break;
            default: type = ConnectionEnum::TYPE_NONE; break;
        }

        m_primary->setName(c->name()).setType(type);
        for (auto &item : m_connections) {
            if (sameConnection(item, *m_primary)) {
                m_primary->setSignalStrength(item.signalStrength);
                break;
            }
        }
    }
    emit primaryChanged();
}

QVariant ConnectionsModel::getPrimary()
{
    return QVariant::fromValue(*m_primary);
}

ConnectionEnum::Action ConnectionsModel::selectActionForItem(const ConnectionItem &item)
{
    for (const auto &ci : m_connections) {
        if (ci.state == ConnectionEnum::STATE_WAIT) {
            if (item.state != ConnectionEnum::STATE_WAIT) return ConnectionEnum::ACTION_NONE;
            break;
        }
    }

    // process only when waiting for a wifi connection
    if (item.state == ConnectionEnum::STATE_WAIT) {
        if (item.type != ConnectionEnum::TYPE_WIRELESS) return ConnectionEnum::ACTION_NONE;
        if (Util::isConnectionCreated(item)) {
            for (const auto &c : NetworkManager::activeConnections()) {
                if (item.path != c->path()) continue;
                if (c->state() != NetworkManager::ActiveConnection::Activating) return ConnectionEnum::ACTION_NONE;
                return ConnectionEnum::ACTION_ABORT_CONNECTING;
            }
        }
        return ConnectionEnum::ACTION_NONE;
    }

    if (item.state == ConnectionEnum::STATE_ON) return ConnectionEnum::ACTION_DISCONNECT;

    // disconnected, but the connection is already configured by someone
    if (Util::isConnectionCreated(item)) return ConnectionEnum::ACTION_CONNECT;

    auto dev = Util::findDeviceForAccessPointPath(item.path);
    if (!dev) return ConnectionEnum::ACTION_ERROR_CANT_FIND_DEVICE;
    if (dev->type() != NetworkManager::Device::Wifi) return ConnectionEnum::ACTION_ERROR_CAN_CREATE_ONLY_WIRELESS;

    auto wifiDev = dev.objectCast<NetworkManager::WirelessDevice>();
    auto ap = wifiDev->findAccessPoint(item.path);
    if (!ap) return ConnectionEnum::ACTION_ERROR_CANT_FIND_AP;

    if (!item.wpaFlags) return ConnectionEnum::ACTION_CONNECT_FREE_WIFI;
    if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmtPsk) return ConnectionEnum::ACTION_CONNECT_WITH_PSK;
    if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmtSAE) return ConnectionEnum::ACTION_CONNECT_WITH_PSK;
    if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmt8021x) return ConnectionEnum::ACTION_CONNECT_8021X_WIFI;

    return ConnectionEnum::ACTION_UNSUPPORTED;
}

void ConnectionsModel::onActionDialogComplete(QVariantMap data)
{
    auto action = data[QStringLiteral("action")].value<ConnectionEnum::Action>();
    auto item = data[QStringLiteral("connection")].value<ConnectionItem>();
    if (item.path.length() == 0) {
        qWarning("%s: empty dbus path, item %s", __FUNCTION__, qPrintable(item.name));
        return;
    }
    switch (action) {
        case ConnectionEnum::ACTION_NONE: break;
        case ConnectionEnum::ACTION_DISCONNECT: disconnectItem(item); break;
        case ConnectionEnum::ACTION_ABORT_CONNECTING: deleteConnection(item); break;
        case ConnectionEnum::ACTION_CONNECT: connectItem(item); break;
        case ConnectionEnum::ACTION_ERROR_RETYPE_PSK:  createAndConnect(data); break;
        case ConnectionEnum::ACTION_CONNECT_FREE_WIFI: createAndConnect(data); break;
        case ConnectionEnum::ACTION_CONNECT_WITH_PSK: createAndConnect(data); break;
        case ConnectionEnum::ACTION_CONNECT_8021X_WIFI: createAndConnect(data); break;
        case ConnectionEnum::ACTION_ERROR_8021X_WIFI: createAndConnect(data); break;
        case ConnectionEnum::ACTION_UNSUPPORTED: break;
        default: qWarning() << __FUNCTION__ << "unhandled action:" << action; break;
    }
}

void ConnectionsModel::createAndConnect(QVariantMap data)
{
    auto item = data[QStringLiteral("connection")].value<ConnectionItem>();

    if (item.path.length() == 0) {
        qWarning("%s: empty dbus path, item %s", __FUNCTION__, qPrintable(item.name));
        return;
    }
    auto wifiDev = Util::findDeviceForAccessPointPath(item.path);
    if (!wifiDev) {
        qWarning("%s: can't find device for access point path: %s", __FUNCTION__, qPrintable(item.path));
        return;
    }
    auto ap = wifiDev->findAccessPoint(item.path);
    if (!ap) {
        qWarning("%s: can't find access point object for path: %s", __FUNCTION__, qPrintable(item.path));
    }

    if (m_username.length() == 0) {
        qWarning("%s: greeter username is empty, can't create a private connection", __FUNCTION__);
    }

    NMVariantMapMap connMap;
    QVariantMap map;
    map.insert(QStringLiteral("id"), ap->ssid());
    QString permissions = QStringLiteral("user:%1").arg(m_username);
    map.insert(QStringLiteral("permissions"), QStringList(permissions));
    map.insert(QStringLiteral("interface-name"), wifiDev->interfaceName());
    map.insert(QStringLiteral("type"), QStringLiteral("802-11-wireless"));
    QVariantMap wirelessMap;
    wirelessMap.insert(QStringLiteral("ssid"), ap->rawSsid());

    if (item.wpaFlags) {
        wirelessMap.insert(QStringLiteral("security"), QStringLiteral("802-11-wireless-security"));

        QVariantMap security;
        QString password = data[QStringLiteral("password")].toString();
        if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmtPsk) {
            security.insert(QStringLiteral("key-mgmt"), QStringLiteral("wpa-psk"));
            security.insert(QStringLiteral("psk"), password);
        } else if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmtSAE) {
            security.insert(QStringLiteral("key-mgmt"), QStringLiteral("sae"));
            security.insert(QStringLiteral("psk"), password);
        } else if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmt8021x) {
            QVariantMap eap;
            eap.insert(QStringLiteral("eap"), QStringList({ QStringLiteral("peap") }));
            eap.insert(QStringLiteral("phase2-auth"), QStringLiteral("mschapv2"));
            eap.insert(QStringLiteral("identity"), data[QStringLiteral("identity")]);
            eap.insert(QStringLiteral("password"), data[QStringLiteral("password")]);
            eap.insert(QStringLiteral("password-flags"), NMSettingSecretFlags::NM_SETTING_SECRET_FLAG_NONE);
            security.insert(QStringLiteral("key-mgmt"), QStringLiteral("wpa-eap"));
            connMap.insert(QStringLiteral("802-1x"), eap);
        } else {
            qWarning() << __FUNCTION__ << "unsupported wpaFlags:" << item.wpaFlags;
            return;
        }
        connMap.insert(QStringLiteral("802-11-wireless-security"), security);
    }

    connMap.insert(QStringLiteral("802-11-wireless"), wirelessMap);
    connMap.insert(QStringLiteral("connection"), map);

    QVariantMap options;
    // the connection will not be saved to disk and will disappear after a disconnect
    options.insert(QStringLiteral("persist"), QStringLiteral("volatile"));
    options.insert(QStringLiteral("bind-activation"), QStringLiteral("dbus-client"));
    m_activator->addAndActivateConnection(connMap, QDBusObjectPath(wifiDev->uni()), QDBusObjectPath(item.path), options);
    connect(m_activator, &ConnectionActivator::wifiKeeperNewConnection, this, [this, wifiDev, item] (QString newPath) {
        Util::waitForDevice(this, wifiDev.data(), [this, item, newPath] {
            if (item.wpaFlags & NetworkManager::AccessPoint::KeyMgmt8021x) {
                emit showDialog(item, ConnectionEnum::ACTION_ERROR_8021X_WIFI);
            } else if (item.wpaFlags) {
                emit showDialog(item, ConnectionEnum::ACTION_ERROR_RETYPE_PSK);
            } else {
                emit showDialog(item, ConnectionEnum::ACTION_FAILED_TO_CONNECT);
            }
        });
    });
}

void ConnectionsModel::deleteConnection(const ConnectionItem &item)
{
    if (!Util::isConnectionCreated(item)) {
        qWarning("%s: can't delete connection because it is not created", __FUNCTION__);
        return;
    }
    auto dev = Util::findDeviceForConnectionPath(item.path);
    if (!dev) {
        qWarning("%s: could not find device for connection: %s", __FUNCTION__, qPrintable(item.path));
        return;
    }
    disconnect(dev.data(), &NetworkManager::Device::stateChanged, nullptr, nullptr);
    auto connection = Util::findConnectionInDevice(item.path, dev);
    if (connection) {
        Util::handleDBusCall(this, connection->remove());
        return;
    }
    qWarning("%s: could not delete connection: %s", __FUNCTION__, qPrintable(item.path));
}
