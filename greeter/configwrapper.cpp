/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "configwrapper.h"

#include "config.h"

#include <QDebug>
#include <QFile>
#include <KSharedConfig>

ConfigWrapper::ConfigWrapper(const QString &groupName, QObject *parent)
    : QObject(parent)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"), KConfig::SimpleConfig);
    m_config = config->group(groupName);
}

QVariant ConfigWrapper::readEntry(const QString &key) const
{
    // to distinguish an empty string from the null string
    if (m_config.hasKey(key)) {
        return m_config.readEntry(key);
    } else {
        return {};
    }
}
