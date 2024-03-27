/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ROOTIMAGEAPP_H
#define ROOTIMAGEAPP_H

#include <QApplication>

class RootImageApp: public QApplication
{
    Q_OBJECT

public:
    RootImageApp(int &argc, char **argv);

private Q_SLOTS:
    void setBackground();
};

#endif /* ROOTIMAGEAPP_H */
