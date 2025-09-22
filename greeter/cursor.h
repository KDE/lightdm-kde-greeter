/*
This file is part of LightDM-KDE.

Copyright (C) 2023-2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CURSOR_H
#define CURSOR_H

#include <QObject>

class Cursor : public QObject
{
    Q_OBJECT

public:
    explicit Cursor(QObject *parent = nullptr);

public Q_SLOTS:
    void move(int x, int y);

private:
    class EIConnection *m_eiConnection;
};

#endif // CURSOR_H
