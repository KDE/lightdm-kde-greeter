/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
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
    return m_config.readEntry(key);
}
