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

#include <QCommandLineParser>
#include <QApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

#include "../about.h"
#include "greeterwindow.h"

int main(int argc, char **argv)
{
    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    KConfigGroup configGroup = config.group("greeter");
    qputenv("QT_IM_MODULE", configGroup.readEntry("input-method", "qtvirtualkeyboard").toUtf8());

    if (configGroup.readEntry("enable-high-dpi", "true") == QStringLiteral("true")) {
        qDebug() << "Enable high DPI scaling and pixmaps.";
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    } else {
        qDebug() << "Don't enable high DPI scaling and pixmaps.";
    }

    QApplication app(argc, argv);
    QCommandLineParser options;

    QString theme = configGroup.readEntry("theme-name", "userbar");
    KLocalizedString::setApplicationDomain((QStringLiteral("lightdm_theme_") + theme).toLocal8Bit().data());

    KAboutData aboutData(
        QStringLiteral("lightdm-kde-greeter"),        // appName
        ki18n("LightDM KDE Greeter").toString(),      // programName
        QStringLiteral("0"),                          // version (set by initAboutData)
        ki18n("Login screen using the LightDM framework").toString(),
        KAboutLicense::GPL);

    initAboutData(&aboutData);
    aboutData.setupCommandLine(&options);

    options.process(app);

    app.setOverrideCursor(QCursor(Qt::ArrowCursor));

    GreeterWindow *w = new GreeterWindow();
    w->show();

    // receive focus
    if (QGuiApplication::primaryScreen() == w->screen()) {
        w->requestActivate();
    }

    return app.exec();
}
