/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef GREETER_WINDOW_H
#define GREETER_WINDOW_H

#include <QQuickView>

class GreeterWrapper;
class QQmlApplicationEngine;

class GreeterWindow: public QObject
{
    Q_OBJECT

public:
    explicit GreeterWindow(QQmlApplicationEngine &engine);
    ~GreeterWindow();

public Q_SLOTS:
    void setRootImage();

private:
    GreeterWrapper *m_greeter;
    bool m_enableRootImageApp = false;
};

#endif // GREETER_WINDOW_H
