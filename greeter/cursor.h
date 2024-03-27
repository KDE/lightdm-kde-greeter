/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CURSOR_H
#define CURSOR_H

#include <QObject>
#include <QCursor>

class Cursor : public QObject
{
    Q_OBJECT

public:
    explicit Cursor(QObject *parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void move(int x, int y) { QCursor::setPos(x, y); }
};

#endif // CURSOR_H
