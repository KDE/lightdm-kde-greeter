/*
This file is part of LightDM-KDE.

Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "eiconnection.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#include <QLoggingCategory>
#include <QSocketNotifier>

#include <libei.h>

static QLoggingCategory lc("EIConnection");
using namespace Qt::StringLiterals;

struct EIConnection::Utils {

    static void logHandler(struct ei * /*ei*/, ei_log_priority priority, const char *message, struct ei_log_context * /*context*/)
    {
        switch (priority) {
        case EI_LOG_PRIORITY_DEBUG:
            qCDebug(lc,) << "Libei:" << message;
            break;
        case EI_LOG_PRIORITY_INFO:
            qCInfo(lc,) << "Libei:" << message;
            break;
        case EI_LOG_PRIORITY_WARNING:
            qCWarning(lc,) << "Libei:" << message;
            break;
        case EI_LOG_PRIORITY_ERROR:
            qCritical(lc) << "Libei:" << message;
            break;
        }
    }
    static void unref(struct ei_device *device)
    {
        if (!device) return;
        ei_device_unref(device);
        device = nullptr;
    }
    static void unref(struct ei *ei)
    {
        if (!ei) return;
        ei_unref(ei);
        ei = nullptr;
    }
};

EIConnection::~EIConnection()
{
    Utils::unref(m_eiDevice);
    Utils::unref(m_eiContext);
}

EIConnection::EIConnection(QObject *parent) :
    QObject(parent),
    m_eiDevice(nullptr),
    m_eiContext(nullptr),
    m_eiSequence(1),
    m_emulating(false)
{
    auto msg = QDBusMessage::createMethodCall(
        u"org.kde.KWin"_s,
        u"/org/kde/KWin/EIS/RemoteDesktop"_s,
        u"org.kde.KWin.EIS.RemoteDesktop"_s,
        u"connectToEIS"_s);

    msg.setArguments({ EI_DEVICE_CAP_POINTER_ABSOLUTE });
    QDBusReply<QDBusUnixFileDescriptor> eisReply = QDBusConnection::sessionBus().call(msg);
    if (!eisReply.isValid()) {
        qCWarning(lc,) << "Can't invoke KWin dbus 'connectToEIS' method, error:" << eisReply.error();
        return;
    }
    int eifd = eisReply.value().takeFileDescriptor();

    m_eiContext = ei_new_sender(nullptr);
    ei_log_set_priority(m_eiContext, EI_LOG_PRIORITY_DEBUG);
    ei_log_set_handler(m_eiContext, Utils::logHandler);

    if(int err = ei_setup_backend_fd(m_eiContext, eifd)) {
        qCWarning(lc,) << "Can't init libei context, error:" << strerror(err);
        return;
    }

    auto esNotifier = new QSocketNotifier(eifd, QSocketNotifier::Read, this);

    m_eiNotifierConnection = connect(esNotifier, &QSocketNotifier::activated, [this](QSocketDescriptor /*socket*/, QSocketNotifier::Type /*type*/){

        ei_dispatch(m_eiContext);

        while (auto event = ei_get_event(m_eiContext)) {
            const auto type = ei_event_get_type(event);

            switch (type) {
            case EI_EVENT_CONNECT:
                qCDebug(lc,) << "Connected to EIS";
                break;
            case EI_EVENT_DISCONNECT:
                qCWarning(lc,) << "Disconnecting from EIS, Cursor::move on Wayland no longer works";
                m_emulating = false;
                Utils::unref(m_eiDevice);
                Utils::unref(m_eiContext);
                disconnect(m_eiNotifierConnection);
                break;
            case EI_EVENT_SYNC:
                break;
            case EI_EVENT_SEAT_ADDED:
                ei_seat_bind_capabilities(ei_event_get_seat(event), EI_DEVICE_CAP_POINTER_ABSOLUTE, nullptr);
                break;
            case EI_EVENT_DEVICE_ADDED: {
                struct ei_device *device = ei_event_get_device(event);
                if (ei_device_has_capability(device, EI_DEVICE_CAP_POINTER_ABSOLUTE)) {
                    qCDebug(lc,) << "New absolute pointer device:" << ei_device_get_name(device);
                    m_eiDevice = ei_device_ref(device);
                }
                break;
            }
            case EI_EVENT_DEVICE_RESUMED:
                if (ei_event_get_device(event) == m_eiDevice) {
                    ei_device_start_emulating(m_eiDevice, m_eiSequence++);
                    m_emulating = true;
                }
                break;
            default:
                qCDebug(lc,) << "Libei event: unexpected event:" << ei_event_type_to_string(type);
            }
            ei_event_unref(event);
        }
    });
}

void EIConnection::moveCursor(int x, int y)
{
    if (!m_emulating) {
        qCDebug(lc,) << "want to move the cursor on Wayland, but the libei emulation is not running";
        return;
    }

    ei_device_pointer_motion_absolute(m_eiDevice, x, y);
    ei_device_frame(m_eiDevice, ei_now(m_eiContext));
}

