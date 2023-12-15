/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#include "configoptions.h"

#include <QFile>
#include <QUiLoader>
#include <QVBoxLayout>
#include <QDebug>

#include <KConfigDialogManager>
#include <KConfigGui/KConfigLoader>


class AuthKitConfigLoader: public KConfigLoader
{
public:
    AuthKitConfigLoader(const KSharedConfigPtr &config, QIODevice *xml, QObject *parent = nullptr);
    QVariantMap entryMap() const;

protected:
    bool usrSave() override;

private:
    QVariantMap m_entryMap;
};

AuthKitConfigLoader::AuthKitConfigLoader(const KSharedConfigPtr &config, QIODevice *xml, QObject *parent)
    : KConfigLoader(config, xml, parent)
{
}

//normal write fails due to needing root, worse it "readConfig" at the end of a write, deleting any values we once had
//we override the usrWrite event to save all settings to entry map then retrieve that.
bool AuthKitConfigLoader::usrSave()
{
    m_entryMap.clear();

    foreach (KConfigSkeletonItem* item, items())
    {
        m_entryMap[QStringLiteral("greeter/greeter-settings/") + item->key()] = item->property();
    }

    return true;
}

QVariantMap AuthKitConfigLoader::entryMap() const
{
    return m_entryMap;
}

ConfigOptions::ConfigOptions(QWidget *parent)
    : QWidget(parent)
{
    new QVBoxLayout(this);
}

void ConfigOptions::setConfig(const KSharedConfigPtr &config)
{
    m_config = config;
}

void ConfigOptions::setTheme(const QDir &themeDir)
{
    if (Q_UNLIKELY(!m_config))
    {
        qFatal("setConfig must be called before setTheme");
    }

    //delete existing widgets.
    if (!m_wrapperWidget.isNull())
    {
        m_wrapperWidget.data()->deleteLater();
    }

    //if contains a valid config
    if (themeDir.exists(QStringLiteral("main.xml")) && themeDir.exists(QStringLiteral("config.ui")))
    {
        QFile kcfgFile(themeDir.filePath(QStringLiteral("main.xml")));
        kcfgFile.open(QFile::ReadOnly);

        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QFile uiFile(themeDir.filePath(QStringLiteral("config.ui")));
        m_wrapperWidget.reset(loader.load(&uiFile, this));

        //both the following get deleted when the wrapped widget is deleted.
        //FIXME I don't really like having so many dangly pointers about...
        m_config->reparseConfiguration();
        m_configLoader = new AuthKitConfigLoader(m_config, &kcfgFile, m_wrapperWidget.data());
        m_manager = new KConfigDialogManager(m_wrapperWidget.data(), m_configLoader);
        connect(m_manager, &KConfigDialogManager::widgetModified, this, &ConfigOptions::onSettingsChanged);

        layout()->addWidget(m_wrapperWidget.data());
    }

    Q_EMIT changed(false);
}

void ConfigOptions::onSettingsChanged()
{
    qDebug() << "changed";
    Q_EMIT changed(true);
}

QVariantMap ConfigOptions::save()
{
    if (m_wrapperWidget.isNull())
    {
        return QVariantMap();
    }

    m_manager->updateSettings();
    return m_configLoader->entryMap();
}

void ConfigOptions::defaults()
{
    if (m_wrapperWidget.isNull())
    {
        return;
    }

    m_manager->updateWidgetsDefault();
}
