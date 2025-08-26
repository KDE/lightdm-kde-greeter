/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef GREETER_CORE_H
#define GREETER_CORE_H

#include <QQuickView>

class GreeterWrapper;
class QQmlApplicationEngine;

class GreeterCore: public QObject
{
    Q_OBJECT

public:
    explicit GreeterCore(QQmlApplicationEngine &engine);
    ~GreeterCore();

public Q_SLOTS:
    void setRootImage();

private:
    GreeterWrapper *m_greeter;
    bool m_enableRootImageApp = false;
};

#endif // GREETER_CORE_H
