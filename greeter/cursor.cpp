/*
This file is part of LightDM-KDE.

Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "cursor.h"

#include <QCursor>
#include <QGuiApplication>
#include <QLoggingCategory>

#include "eiconnection.h"

static QLoggingCategory lc("Cursor");
using namespace Qt::StringLiterals;

Cursor::Cursor(QObject *parent) :
    QObject(parent),
    m_eiConnection(nullptr)
{
    if (QGuiApplication::platformName() == u"wayland"_s) {
        m_eiConnection = new EIConnection(this);
    }
}

void Cursor::move(int x, int y)
{
    if (m_eiConnection) {
        m_eiConnection->moveCursor(x, y);
    } else {
        QCursor::setPos(x, y);
        return;
    }
}
