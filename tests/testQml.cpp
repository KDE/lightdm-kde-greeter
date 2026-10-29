/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDir>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include "connectionenum.h"
#include "mock_accessibilitybackend.h"
#include "mock_qdbus.h"
#include "greeterwrapper.h"

using namespace Qt::StringLiterals;

static const QString s_lightdmGroup = u"lightdm"_s;
static const QString s_lastUsersKey = u"lastUser"_s;
static const QString s_lastSessionsKey = u"lastSession"_s;

static const QString s_systemdService =          u"org.freedesktop.systemd1"_s;
static const QString s_systemdPath =             u"/org/freedesktop/systemd1"_s;
static const QString s_screenReaderUnitPath =    u"/org/freedesktop/systemd1/unit/orca_2eservice"_s;
static const QString s_unitInterface =           u"org.freedesktop.systemd1.Unit"_s;
static const QString s_managerInterface =        u"org.freedesktop.systemd1.Manager"_s;
static const QString s_propertiesInterface =     u"org.freedesktop.DBus.Properties"_s;

class Setup : public QObject
{
    Q_OBJECT

public:
    Setup() {}

public Q_SLOTS:
    void applicationAvailable()
    {
        qmlRegisterUncreatableMetaObject(ConnectionEnum::staticMetaObject, "ConnectionEnum", 1, 0, "ConnectionEnum", QStringLiteral("Error: only enums"));
    }

    void initTestCase()
    {
        m_tempDir = QDir::tempPath() + u"/tst_qml_greeter_"_s + QString::number(QCoreApplication::applicationPid());
        QVERIFY(QDir().mkpath(m_tempDir));
        qputenv("XDG_CONFIG_HOME", m_tempDir.toUtf8());
        QStandardPaths::setTestModeEnabled(true);
    }

    void qmlEngineAvailable(QQmlEngine *engine)
    {
        m_engine = engine;
        engine->rootContext()->setContextProperty(u"mockAccessibility"_s, &m_backend);
        // Expose this Setup object to QML so tests can call resetGreeter().
        engine->rootContext()->setContextProperty(u"testSetup"_s, this);
    }

    void cleanupTestCase()
    {
        qunsetenv("XDG_CONFIG_HOME");
        QStandardPaths::setTestModeEnabled(false);
        if (!m_tempDir.isEmpty()) {
            QVERIFY(QDir(m_tempDir).removeRecursively());
        }
    }

public:
    Q_INVOKABLE void resetGreeter(const QString &lastUsers, const QString &lastSessions)
    {
        if (!m_engine) return;

        // Write state to config
        KSharedConfig::Ptr config = KSharedConfig::openConfig(u"state-kde"_s);
        KConfigGroup group(config, s_lightdmGroup);
        if (lastUsers.isEmpty()) {
            group.deleteEntry(s_lastUsersKey);
        } else {
            group.writeEntry(s_lastUsersKey, lastUsers);
        }
        if (lastSessions.isEmpty()) {
            group.deleteEntry(s_lastSessionsKey);
        } else {
            group.writeEntry(s_lastSessionsKey, lastSessions);
        }
        config->sync();

        // Destroy the old GreeterWrapper synchronously before clearing the D-Bus mocks.
        // Its child QDBusPendingCallWatcher objects must be destroyed while the mock
        // services they reference are still alive (their destructors remove themselves
        // from the service's watcher list).
        if (m_greeter) {
            delete m_greeter;
            m_greeter = nullptr;
        }

        // Register D-Bus mocks
        auto &bus = QDBusConnection::sessionBus();
        bus.clearMockServices();
        bus.addMockService(u"manager"_s, mock_key(s_systemdService, s_systemdPath, s_managerInterface));
        bus.addMockService(u"orcaUnit"_s, mock_key(s_systemdService, s_screenReaderUnitPath, s_unitInterface));
        bus.addMockService(u"properties"_s, mock_key(s_systemdService, s_screenReaderUnitPath, s_propertiesInterface));

        // Create a fresh GreeterWrapper that reads the just-written state.
        m_greeter = new GreeterWrapper(nullptr);
        m_engine->rootContext()->setContextProperty(u"greeter"_s, m_greeter);
    }

private:
    QString m_tempDir;
    MockBackend m_backend;
    QQmlEngine *m_engine = nullptr;
    GreeterWrapper *m_greeter = nullptr;
};

QUICK_TEST_MAIN_WITH_SETUP(testQml, Setup)

#include "testQml.moc"
