/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MOCK_QDBUS_H
#define MOCK_QDBUS_H

#include <QLoggingCategory>
#include <QString>
#include <QVariant>
#include <QVariantMap>
#include <QStringList>
#include <QObject>
#include <functional>
#include <memory>

using namespace Qt::StringLiterals;
static const QLoggingCategory mock_qdbus("MockQDBus");

class QDBusPendingCallWatcher;
class QDBusPendingCall;
class QDBusInterface;

class QDBusError {
public:
    enum ErrorType {
        NoError,
        Other,
        Failed,
        InvalidInterface
    };

    QDBusError() : m_type(NoError), m_message() {}
    QDBusError(ErrorType type, const QString &message) : m_type(type), m_message(message) {}

    ErrorType type() const { return m_type; }
    QString message() const { return m_message; }

    bool isValid() const { return m_type != NoError; }

private:
    ErrorType m_type;
    QString m_message;
};

class QDBusObjectPath {
public:
    QDBusObjectPath() = default;
    explicit QDBusObjectPath(const QString &path) : m_path(path) {}
    explicit QDBusObjectPath(const char *path) : m_path(QString::fromUtf8(path)) {}

    QString path() const { return m_path; }
    operator QString() const { return m_path; }
    const QString &toString() const { return m_path; }

private:
    QString m_path;
};

class QDBusMockService;

class QDBusPendingCall {
public:

    QDBusPendingCall(const QString &errorMessage)
        : m_error{ QDBusError::Other, errorMessage } {}

    QDBusPendingCall(const QString &method, const QVariantList &argList, QDBusMockService *callee)
        : m_method(method), m_argList(argList), m_callee(callee)
    {
        qCDebug(mock_qdbus,) << "new pending call for method:" << method << "args" << argList;
    }

    template <typename T>
    QDBusPendingCall(T value)
        : m_value(QVariant::fromValue(value)) {}

    bool isError() const { return m_error.isValid(); }
    void setError(QDBusError error)
    {
        m_error = error;
    }

    QDBusError error() const { return m_error; }

    QDBusMockService *callee() { return m_callee; }
    QString getMethod() const { return m_method; }
    QVariantList getArgList() const { return m_argList; }
    QVariant getValue() const { return m_value; }

private:

    QDBusError m_error;
    QVariant m_value;

    QString m_method;
    QVariantList m_argList;

    QDBusMockService *m_callee;
};

class QDBusMockService {

public:
    struct Subscriber {
        QString signal;
        QObject *receiver;
        const char *slot;
    };

public:
    auto &subscribers() { return m_subscribers; }
    auto &calls() { return m_calls; }
    auto &watchers() { return m_watchers; }
    auto &properties() { return m_properties; }

    QDBusPendingCallWatcher *watcher(int idx) {
        if (idx >= m_watchers.size()) return nullptr;
        return m_watchers[idx];
    }

    Subscriber *subscriber(int idx) {
        if (idx >= m_subscribers.size()) return nullptr;
        return &m_subscribers[idx];
    }

    bool isValid() { return m_valid; }
    void setValid(bool value) { m_valid = value; }

private:

    QList<Subscriber> m_subscribers;
    QList<QDBusPendingCall> m_calls;
    QList<QDBusPendingCallWatcher*> m_watchers;
    QMap<QString, QString> m_properties;

    bool m_valid = true;
};

static QString mock_key(const QString &service, const QString &path, const QString interface) {
    return service + u" "_s + path + u" "_s + interface;
}

class QDBusConnection {
public:
    static QDBusConnection &sessionBus()
    {
        static QDBusConnection sessionBus;
        return sessionBus;
    }

    bool isConnected() const { return m_connected; }
    QDBusError lastError() const { return m_error; }

    bool connect(const QString &service, const QString &path,
                 const QString &interface, const QString &signal,
                 QObject *receiver, const char *slot) {

        QString key = mock_key(service, path, interface);

        qCDebug(mock_qdbus,) << "subscribing for signal" << signal << "slot" << slot << "of interface" << key;

        if (m_shared->mocks.contains(key) && m_shared->mocks[key].isValid()) {
            m_shared->mocks[key].subscribers().append({ signal, receiver, slot });
            return true;
        } else {
            m_error = QDBusError(QDBusError::ErrorType::InvalidInterface, u"no such mock interface: %1"_s.arg(key) );
            return false;
        }
    }

    QDBusMockService &addMockService(const QString &alias, const QString &key, QDBusMockService &&mock = {}) {
        m_shared->mocks[key] = std::move(mock);
        m_shared->aliases[alias] = key;
        return m_shared->mocks[key];
    }

    void clearMockServices()
    {
        m_shared->mocks.clear();
        m_shared->aliases.clear();
    }

    QDBusMockService *mockServiceWithKey(const QString &key)
    {
        auto it = m_shared->mocks.find(key);
        if (it != m_shared->mocks.end()) {
            return &it.value();
        }
        return nullptr;
    }

    QDBusMockService *mockService(const QString &alias)
    {
        auto it = m_shared->aliases.find(alias);
        if (it != m_shared->aliases.end()) {
            return mockServiceWithKey(it.value());
        }
        return nullptr;
    }

private:

    using MockServiceContainer = QMap<QString, QDBusMockService>;

    bool m_connected = true;
    QDBusError m_error;

    struct SharedData {
        QMap<QString, QDBusMockService> mocks;
        QMap<QString, QString> aliases;
    };
    QSharedPointer<SharedData> m_shared{ new SharedData };
};

template <typename... Ts>
class QDBusPendingReply;

template <typename T>
class QDBusPendingReply<T> : public QDBusPendingCall {

public:
    QDBusPendingReply(const QDBusPendingCall &call)
        : QDBusPendingCall(call)
        , m_value(call.getValue().value<T>())
    {}

    T value() const { return m_value; }
    operator T() const { return m_value; }

private:
    T m_value;
};

template <>
class QDBusPendingReply<> : public QDBusPendingCall {
public:
    QDBusPendingReply(const QDBusPendingCall &call) : QDBusPendingCall(call) {}
};

class QDBusInterface {
public:
    QDBusInterface(const QString &service, const QString &path,
                   const QString &interface, QDBusConnection &connection)
        : m_service(service), m_path(path), m_interface(interface) {

            auto key = getKey();
            m_mock = connection.mockServiceWithKey(key);
            if (!m_mock) {
                m_isValid = false;
                m_lastError = QDBusError(QDBusError::ErrorType::InvalidInterface, u"no such mock interface: %1"_s.arg(key));
            } else if (!m_mock->isValid()) {
                m_isValid = false;
                m_lastError = QDBusError(QDBusError::ErrorType::InvalidInterface, u"set via mock service"_s);
            } else {
                m_isValid = true;
            }
    }

    bool isValid() const { return m_mock && m_isValid; }
    QDBusError lastError() const { return m_lastError; }

    QString getKey() {
        return mock_key(m_service, m_path, m_interface);
    }

    QVariant property(const QString &name) const {
        if (!isValid()) qCFatal(mock_qdbus,) << "using of invalid interface";
        return m_mock->properties().value(name);
    }

    QVariant property(const char *name) const {
        return property(QString::fromUtf8(name));
    }

    template <typename... Args>
    QDBusPendingCall asyncCall(QString methodName, Args&&... args) {

        if (!isValid()) qCFatal(mock_qdbus,) << "using of invalid interface";

        QVariantList argList;

        (argList.append(QVariant::fromValue(std::forward<Args>(args))), ...);
        m_mock->calls().push_back(QDBusPendingCall(methodName, argList, m_mock));

        return m_mock->calls().back();
    }

    void setValid(bool valid) { m_isValid = valid; }
    void setLastError(const QDBusError &err) { m_lastError = err; }

    QDBusMockService *mock() { return m_mock; }

private:
    QString m_service;
    QString m_path;
    QString m_interface;

    QDBusError m_lastError;
    bool m_isValid;

    QDBusMockService *m_mock;
};

class QDBusPendingCallWatcher : public QObject, public QDBusPendingCall
{
    Q_OBJECT
public:
    QDBusPendingCallWatcher(const QDBusPendingCall &reply, QObject *parent)
        : QObject(parent), QDBusPendingCall(reply)
    {
        callee()->watchers().append(this);
    }

    ~QDBusPendingCallWatcher()
    {
        callee()->watchers().removeAll(this);
    }

    void triggerFinished() {
        Q_EMIT finished(this);
    }

Q_SIGNALS:
    void finished(QDBusPendingCallWatcher *watcher);
};

#endif // MOCK_QDBUS_H
