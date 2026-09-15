/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "accessibility.h"

#include "greeterwrapper.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QLoggingCategory>

#define TRANSLATION_DOMAIN "lightdm_kde_greeter"
#include <KLocalizedString>

static const QLoggingCategory lc("Accessibility");
using namespace Qt::StringLiterals;

static const QString s_systemdService =          u"org.freedesktop.systemd1"_s;

static const QString s_systemdPath =             u"/org/freedesktop/systemd1"_s;
static const QString s_screenReaderUnitPath =    u"/org/freedesktop/systemd1/unit/orca_2eservice"_s;
static const QString s_screenReaderServiceName = u"orca.service"_s;

static const QString s_unitInterface =           u"org.freedesktop.systemd1.Unit"_s;
static const QString s_managerInterface =        u"org.freedesktop.systemd1.Manager"_s;
static const QString s_propertiesInterface =     u"org.freedesktop.DBus.Properties"_s;

Accessibility::Accessibility(GreeterWrapper *parent)
    : QObject(parent)
    , m_screenReaderActive(false)
{
    subscribeOnDBusSignals();
    queryScreenReaderState();
}

void Accessibility::setScreenReaderActive(bool active)
{
    if (m_screenReaderActive == active) return;

    active ? enableScreenReader() : disableScreenReader();
}

void Accessibility::reportScreenReaderActive(bool active, bool force)
{
    if (m_screenReaderActive == active && !force) return;

    m_screenReaderActive = active;
    Q_EMIT screenReaderActiveChanged();
}

void Accessibility::handleDBusSignal(const QString &interfaceName, const QVariantMap &props, const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);

    if (interfaceName == s_unitInterface) {

        auto value = props.value(u"ActiveState"_s);

        if (value.isValid()) {
            auto state = value.toString();
            if (state == u"active"_s) reportScreenReaderActive(true);
            else if (state == u"inactive"_s) reportScreenReaderActive(false);
            else if (state == u"failed"_s) {
                reportScreenReaderActive(false, true);
                if (m_screenReaderPreviousState == u"activating"_s) {
                    Q_EMIT screenReaderBackendError(ki18n("Screen reading service failed to start.").toString());
                }
            }
            m_screenReaderPreviousState = state;
        }
    }
}

void Accessibility::queryScreenReaderState()
{
    QDBusInterface qiface(s_systemdService, s_screenReaderUnitPath, s_unitInterface, QDBusConnection::sessionBus());

    if (!qiface.isValid()) {
        qCWarning(lc,) << "failed to init screen reader unit interface:" << qiface.lastError().message();
        return;
    }

    QVariant result = qiface.property("ActiveState");

    if (!result.isValid()) {
        qCWarning(lc,) << "failed to get 'ActiveState' property from screen reader unit interface";
        return;
    }

    bool screenReaderActive = result.toString() == u"active"_s;
    reportScreenReaderActive(screenReaderActive);

    QString screenReaderOption = static_cast<GreeterWrapper*>(parent())->getAccessibilitySetting(u"screenReader"_s);
    bool screenReaderWantActive = screenReaderOption == u"enabled"_s;

    if (screenReaderActive != screenReaderWantActive) setScreenReaderActive(screenReaderWantActive);
}

void Accessibility::subscribeOnDBusSignals()
{
    QDBusInterface qiface(s_systemdService,
                          s_systemdPath,
                          s_managerInterface,
                          QDBusConnection::sessionBus());

    if (!qiface.isValid()) {
        qCWarning(lc,) << "invalid dbus interface" << qiface.lastError().message();
        return;
    }

    QDBusPendingReply<> reply = qiface.asyncCall(u"Subscribe"_s);

    auto pendingCallWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<> reply = *w;
        if (reply.isError()) {
            qCWarning(lc,) << "failed to subscribe on systemd manager" << reply.error().message();
        } else {
            subscribeOnScreenReaderState();
        }
    });
}

void Accessibility::subscribeOnScreenReaderState()
{
    QDBusConnection bus = QDBusConnection::sessionBus();

    if (!bus.isConnected()) {
        qCWarning(lc,) << "failed to connect to DBus" << bus.lastError().message();
    }

    bool success = bus.connect(s_systemdService,
                               s_screenReaderUnitPath,
                               s_propertiesInterface,
                               u"PropertiesChanged"_s,
                               this, SLOT(handleDBusSignal(QString, QVariantMap, QStringList)));
    if (!success) {
        qCWarning(lc,) << "failed to subscribe:" << bus.lastError().message();
    } else {
        qCInfo(lc,) << "successfully subscribed to PropertiesChanged";
    }
}

void Accessibility::enableScreenReader()
{
    QDBusInterface qiface(s_systemdService, s_systemdPath, s_managerInterface, QDBusConnection::sessionBus());

    QDBusPendingReply<QDBusObjectPath> reply = qiface.asyncCall(u"RestartUnit"_s, s_screenReaderServiceName, u"replace"_s);
    auto pendingCallWatcher = new QDBusPendingCallWatcher(reply, this);
    connect(pendingCallWatcher, &QDBusPendingCallWatcher::finished, this, [this] (QDBusPendingCallWatcher *w) {
        w->deleteLater();
        QDBusPendingReply<QDBusObjectPath> reply = *w;
        if (reply.isError()) {
            qCWarning(lc,) << "failed to start screen reader unit:" << reply.error().message();
            Q_EMIT screenReaderBackendError(ki18n("Screen reading service not found.").toString());
            reportScreenReaderActive(false, true);
        } else {
            static_cast<GreeterWrapper*>(parent())->setAccessibilitySetting(u"screenReader"_s, u"enabled"_s);
        }
    });
}

void Accessibility::disableScreenReader()
{
    QDBusInterface qiface(s_systemdService, s_systemdPath, s_managerInterface, QDBusConnection::sessionBus());
    qiface.asyncCall(u"StopUnit"_s, s_screenReaderServiceName, u"replace"_s);
    static_cast<GreeterWrapper*>(parent())->setAccessibilitySetting(u"screenReader"_s, u"disabled"_s);
}
