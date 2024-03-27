/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef CONFIGWRAPPER_H
#define CONFIGWRAPPER_H

#include <QDir>
#include <QObject>
#include <QString>

#include <KConfigGroup>

/** This class exposes the lightdm-kde config to QML*/

class ConfigWrapper: public QObject
{
    Q_OBJECT

public:
    explicit ConfigWrapper(const QString &kcfgPath, QObject *parent = nullptr);

    Q_INVOKABLE QVariant readEntry(const QString &key) const;

private:
    KConfigGroup m_config;
};

#endif // CONFIGWRAPPER_H
