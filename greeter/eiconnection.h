/*
This file is part of LightDM-KDE.

Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef EICONNECTION_H
#define EICONNECTION_H

#include <QObject>

class EIConnection : public QObject
{
    Q_OBJECT

public:

    EIConnection(QObject *parent);
    void moveCursor(int x, int y);
    ~EIConnection();

private:
    struct Utils;

    struct ei_device *m_eiDevice;
    struct ei *m_eiContext;
    int m_eiSequence;
    bool m_emulating;
    QMetaObject::Connection m_eiNotifierConnection;
};


#endif // EICONNECTION_H
