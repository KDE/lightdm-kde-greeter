/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QtTest>
#include <QObject>

#include "mock_qdbus.h"

#include "accessibility.h"
#include "mock_greeterwrapper.h"

using namespace Qt::StringLiterals;

static const QString s_systemdService =          u"org.freedesktop.systemd1"_s;

static const QString s_systemdPath =             u"/org/freedesktop/systemd1"_s;
static const QString s_screenReaderUnitPath =    u"/org/freedesktop/systemd1/unit/orca_2eservice"_s;
static const QString s_screenReaderServiceName = u"orca.service"_s;

static const QString s_unitInterface =           u"org.freedesktop.systemd1.Unit"_s;
static const QString s_managerInterface =        u"org.freedesktop.systemd1.Manager"_s;
static const QString s_propertiesInterface =     u"org.freedesktop.DBus.Properties"_s;

class TestAccessibility : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
    }

    void cleanupTestCase()
    {
    }

    void init()
    {
        auto bus = QDBusConnection::sessionBus();
        bus.clearMockServices();

        bus.addMockService(u"manager"_s, mock_key(s_systemdService, s_systemdPath, s_managerInterface));
        bus.addMockService(u"orcaUnit"_s, mock_key(s_systemdService, s_screenReaderUnitPath, s_unitInterface));
        bus.addMockService(u"properties"_s, mock_key(s_systemdService, s_screenReaderUnitPath, s_propertiesInterface));
    }

    bool triggerWatcher(const QString &mockServiceName, const QString &methodName, const QDBusError error = {})
    {
        QDBusConnection bus = QDBusConnection::sessionBus();

        auto mockService = bus.mockService(mockServiceName);
        if (!mockService) {
            qWarning() << __FUNCTION__ << "- mock service not found:" << mockServiceName;
            return false;
        }

        // triggering the last watcher for the method, if has
        QDBusPendingCallWatcher *watcher = nullptr;
        for (int i = mockService->watchers().size() - 1; i >= 0; --i) {
            auto w = mockService->watchers()[i];
            if (w->getMethod() == methodName) {
                watcher = w;
                break;
            }
        }

        if (!watcher) {
            qWarning() << __FUNCTION__ << "- no watcher for methodName:" << methodName;
            return false;
        }

        if (error.isValid()) {
            watcher->setError(error);
        }
        watcher->triggerFinished();
        return true;
    }

    bool checkLastCall(const QString &mockServiceName, const QString &methodName)
    {

        QDBusConnection bus = QDBusConnection::sessionBus();

        auto mockService = bus.mockService(mockServiceName);
        if (!mockService) {
            qWarning() << __FUNCTION__ << "- mock service not found:" << mockServiceName;
            return false;
        }

        if (mockService->calls().size() <= 0) {
            qWarning() << __FUNCTION__ << "- no calls at mock service:" << mockServiceName;
            return false;
        }

        auto actualMethodName = mockService->calls().last().getMethod();
        if (actualMethodName != methodName)
        {
            qWarning() << __FUNCTION__ << "- method name does not match, expected:" << methodName << "actual:" << actualMethodName;
            return false;
        }
        return true;
    }

    void testAccessibilityInstantiation()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();

        auto orca = bus.mockService(u"orcaUnit"_s);
        QVERIFY(orca);
        orca->properties()[u"ActiveState"_s] = u"inactive"_s;

        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);
        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));

        // should be call subscribeOnScreenReaderState
        auto props = bus.mockService(u"properties"_s);
        QVERIFY(props);
        auto propSubscriber = props->subscriber(0);
        QVERIFY(propSubscriber);
        QCOMPARE(propSubscriber->signal, u"PropertiesChanged"_s);

        QVERIFY(accessibility.isScreenReaderActive() == false);
    }

    void testScreenReaderActiveProperty()
    {
        auto bus = QDBusConnection::sessionBus();

        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);
        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));

        QSignalSpy spy(&accessibility, &Accessibility::screenReaderActiveChanged);

        accessibility.setScreenReaderActive(true);
        QVERIFY(triggerWatcher(u"manager"_s, u"RestartUnit"_s));
        QCOMPARE(greeter.getAccessibilitySetting(u"screenReader"_s), u"enabled"_s);

        QCOMPARE(accessibility.isScreenReaderActive(), false);

        QVariantMap propMap;
        propMap[u"ActiveState"_s] = u"active"_s;
        accessibility.handleDBusSignal(s_unitInterface, propMap, QStringList{});

        QCOMPARE(accessibility.isScreenReaderActive(), true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(greeter.getAccessibilitySetting(u"screenReader"_s), u"enabled"_s);

        spy.clear();
        accessibility.setScreenReaderActive(false);

        QVERIFY(checkLastCall(u"manager"_s, u"StopUnit"_s));
        QCOMPARE(accessibility.isScreenReaderActive(), true);

        propMap[u"ActiveState"_s] = u"inactive"_s;
        accessibility.handleDBusSignal(s_unitInterface, propMap, QStringList{});

        QCOMPARE(accessibility.isScreenReaderActive(), false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(greeter.getAccessibilitySetting(u"screenReader"_s), u"disabled"_s);
    }

    void testHandleDBusSignalFailed()
    {
        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);
        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));

        QSignalSpy spy(&accessibility, &Accessibility::screenReaderActiveChanged);
        QSignalSpy errorSpy(&accessibility, &Accessibility::screenReaderBackendError);

        QVariantMap propMap;
        propMap[u"ActiveState"_s] = u"failed"_s;
        accessibility.handleDBusSignal(s_unitInterface, propMap, QStringList{});

        QCOMPARE(accessibility.isScreenReaderActive(), false);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(errorSpy.count(), 0); // No error if not transitioning from activating
    }

    void testHandleDBusSignalFailedFromActivating()
    {
        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);

        QSignalSpy spy(&accessibility, &Accessibility::screenReaderActiveChanged);
        QSignalSpy errorSpy(&accessibility, &Accessibility::screenReaderBackendError);

        // First set to activating state
        QVariantMap propMap;
        propMap[u"ActiveState"_s] = u"activating"_s;
        accessibility.handleDBusSignal(s_unitInterface, propMap, QStringList{});

        // Then fail
        propMap.insert(u"ActiveState"_s, u"failed"_s);
        accessibility.handleDBusSignal(u"org.freedesktop.systemd1.Unit"_s, propMap, QStringList());

        QCOMPARE(accessibility.isScreenReaderActive(), false);
        QCOMPARE(errorSpy.count(), 1);
    }

    void testAutoEnableScreenReader()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();

        auto orca = bus.mockService(u"orcaUnit"_s);
        QVERIFY(orca);
        orca->properties()[u"ActiveState"_s] = u"inactive"_s;

        GreeterWrapper greeter;
        greeter.setAccessibilitySetting(u"screenReader"_s, u"enabled"_s);

        Accessibility accessibility(&greeter);

        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));
        QVERIFY(triggerWatcher(u"manager"_s, u"RestartUnit"_s));
    }

    void testAutoDisableScreenReader()
    {
        QDBusConnection bus = QDBusConnection::sessionBus();

        auto orca = bus.mockService(u"orcaUnit"_s);
        QVERIFY(orca);
        orca->properties()[u"ActiveState"_s] = u"active"_s;

        GreeterWrapper greeter;
        greeter.setAccessibilitySetting(u"screenReader"_s, u"disabled"_s);

        Accessibility accessibility(&greeter);

        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));
        QVERIFY(checkLastCall(u"manager"_s, u"StopUnit"_s));
    }

    void testInvalidManagerInterface()
    {
        auto bus = QDBusConnection::sessionBus();

        auto manager = bus.mockService(u"manager"_s);
        manager->setValid(false);

        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);

        QVERIFY(!checkLastCall(u"manager"_s, u"Subscribe"_s));
    }

    void testNoScreenReaderInSystem()
    {
        auto bus = QDBusConnection::sessionBus();

        bus.mockService(u"orcaUnit"_s)->setValid(false);

        auto props = bus.mockService(u"properties"_s);
        props->setValid(false);

        GreeterWrapper greeter;
        Accessibility accessibility(&greeter);

        QVERIFY(triggerWatcher(u"manager"_s, u"Subscribe"_s));

        // must be failed to subscribe
        QVERIFY(!props->subscriber(0));

        QSignalSpy errorSpy(&accessibility, &Accessibility::screenReaderBackendError);

        // try to enable screen reader
        accessibility.setScreenReaderActive(true);

        // method 'RestartUnit' is called anyway, but it will return error
        QVERIFY(triggerWatcher(u"manager"_s, u"RestartUnit"_s, { QDBusError::Failed, u"no interface for testing purposes"_s }));
        QCOMPARE_NE(greeter.getAccessibilitySetting(u"screenReader"_s), u"enabled"_s);
        QCOMPARE(accessibility.isScreenReaderActive(), false);
        // error sent to user
        QCOMPARE(errorSpy.count(), 1);
    }
};

QTEST_MAIN(TestAccessibility)

#include "tst_Accessibility.moc"
