/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TST_ACCESSIBILITY_H
#define TST_ACCESSIBILITY_H

#include "mock_qdbus.h"

#include <QObject>
#include <QString>

class TestAccessibility : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void testAccessibilityInstantiation();
    void testScreenReaderActiveProperty();
    void testHandleDBusSignalFailed();
    void testHandleDBusSignalFailedFromActivating();
    void testAutoEnableScreenReader();
    void testAutoDisableScreenReader();
    void testInvalidManagerInterface();
    void testNoScreenReaderInSystem();

private:
    bool triggerWatcher(const QString &mockServiceName, const QString &methodName, const QDBusError error = {});
    bool checkLastCall(const QString &mockServiceName, const QString &methodName);
};

#endif // TST_ACCESSIBILITY_H
