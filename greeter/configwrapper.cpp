/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

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

ConfigWrapper::ConfigWrapper(const QString &kcfgPath, QObject *parent)
    : QObject(parent)
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"), KConfig::SimpleConfig);

    QFile xmlFile(kcfgPath);
    xmlFile.open(QFile::ReadOnly);

    m_config = new KConfigLoader(config, &xmlFile, this);
}

QVariant ConfigWrapper::readEntry(const QString &key) const
{
    //FIXME I should use a KConfigSkeleton which loads the KCFG, then remove the "default" parameter

    return m_config->property(key);
}
