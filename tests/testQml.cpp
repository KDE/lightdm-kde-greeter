/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QtQuickTest>
#include <QQmlEngine>
#include <QQmlContext>
#include "connectionenum.h"
#include "mock_accessibilitybackend.h"

using namespace Qt::StringLiterals;

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

    void qmlEngineAvailable(QQmlEngine *engine)
    {
        engine->rootContext()->setContextProperty(u"mockAccessibility"_s, &m_backend);
    }

    void cleanupTestCase()
    {
    }

private:
    MockBackend m_backend;
};

QUICK_TEST_MAIN_WITH_SETUP(testQml, Setup)

#include "testQml.moc"
