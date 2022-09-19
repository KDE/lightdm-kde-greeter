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
#include "lightdmkcm.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QTabWidget>

#include <KAboutData>
#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KConfigDialogManager>
#include <KLocalizedString>
#include <KPluginFactory>

#include "../about.h"
#include "themeconfig.h"
#include "coreconfig.h"

K_PLUGIN_FACTORY_WITH_JSON(LightDMKcmFactory, "kcm_lightdm.json", registerPlugin<LightDMKcm>();)

LightDMKcm::LightDMKcm(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    KAboutData* aboutData = new KAboutData(
        QStringLiteral("kcmlightdm"),                // appName
        ki18n("LightDM KDE Config").toString(), // programName
        QStringLiteral("0"),                        // version (set by initAboutData)
        ki18n("Login screen using the LightDM framework").toString(),
        KAboutLicense::GPL);

    initAboutData(aboutData);

    setAboutData(aboutData);

    setNeedsAuthorization(true);

    QHBoxLayout *layout = new QHBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);

    m_coreConfig = new CoreConfig(this);
    m_themeConfig = new ThemeConfig(this);

    connect(m_themeConfig, &ThemeConfig::changed, this, &LightDMKcm::changed);
    connect(m_coreConfig, &CoreConfig::changed, this, &LightDMKcm::changed);

    tabWidget->addTab(m_themeConfig, i18n("Theme"));
    tabWidget->addTab(m_coreConfig, i18n("General"));
}

void LightDMKcm::save()
{
    QVariantMap args;

    args = m_themeConfig->save();
    args.insert(m_coreConfig->save());

    KAuth::Action saveAction(QStringLiteral("org.kde.kcontrol.kcmlightdm.save"));
    saveAction.setHelperId(QStringLiteral("org.kde.kcontrol.kcmlightdm"));
    saveAction.setArguments(args);
    KAuth::ExecuteJob *job = saveAction.execute();
    if (!job->exec())
    {
        // FIXME: Show a message here
        qWarning() << "save failed:" << job->errorText() << ", " << job->errorString();
    }
    else
    {
        changed(false);
    }
}

void LightDMKcm::defaults()
{
    m_themeConfig->defaults();
}

#include "lightdmkcm.moc"
