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
#include "greeterwindow.h"

#include <QtWidgets/QWidget>
#include <QApplication>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QDesktopWidget>
#include <QDir>
#include <QPixmap>
#include <QProcess>
#include <QtCore/QDebug>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QScreen>
#include <QUrl>
#include <QStandardPaths>
#include <QDebug>

#include <QLightDM/Power>

#include <KDeclarative/KDeclarative>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <Plasma/Theme>

#include "extrarowproxymodel.h"
#include "faceimageprovider.h"
#include "configwrapper.h"
#include "sessionsmodel.h"
#include "usersmodel.h"
#include "screensmodel.h"
#include "greeterwrapper.h"

#include <config.h>

GreeterWindow::GreeterWindow(QWindow *parent)
    : QQuickView(parent),
      m_greeter(new GreeterWrapper(this))
{
    QRect screen = QApplication::desktop()->rect();
    setGeometry(screen);
    
    KDeclarative::KDeclarative::setupEngine(engine());

    UsersModel* usersModel = new UsersModel(this);

    if (m_greeter->hasGuestAccountHint()) {
        usersModel->setShowGuest(true);
    }

    engine()->addImageProvider("face", new FaceImageProvider(usersModel));

    KConfig config(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf");
    KConfigGroup configGroup = config.group("greeter");

    QString theme = configGroup.readEntry("theme-name", "userbar");
    QUrl source = QStandardPaths::locate(QStandardPaths::AppDataLocation, "themes/" + theme + "/main.qml");

    if (source.isEmpty()) {
        qCritical() << "Cannot find QML file for" << theme << "theme. Falling back to \"userbar\" theme.";
        theme = "userbar";
        source = QStandardPaths::locate(QStandardPaths::AppDataLocation, "themes/userbar/main.qml");
        if (source.isEmpty()) {
            qFatal("Cannot find QML file for \"userbar\" theme. Something is wrong with this installation. Aborting.");
        }
    }
    qDebug() << "Loading" << source;

    KLocalizedString::setApplicationDomain((QStringLiteral("lightdm_theme_") + theme).toLocal8Bit().data());

    rootContext()->setContextProperty("config", new ConfigWrapper(QStandardPaths::locate(QStandardPaths::AppDataLocation, "themes/" + theme + "/main.xml"), this));
    rootContext()->setContextProperty("screenSize", size());
    rootContext()->setContextProperty("greeter", m_greeter);
    rootContext()->setContextProperty("usersModel", usersModel);
    rootContext()->setContextProperty("sessionsModel", new SessionsModel(this));
    rootContext()->setContextProperty("screensModel", new ScreensModel(this));
    rootContext()->setContextProperty("power", new QLightDM::PowerInterface(this));
    rootContext()->setContextProperty("plasmaTheme", new Plasma::Theme(this));

    setSource(source);

    connect(m_greeter, &GreeterWrapper::aboutToLogin, this, &GreeterWindow::setRootImage);
}

GreeterWindow::~GreeterWindow()
{
}

void GreeterWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    rootContext()->setContextProperty("screenSize", event->size());
}

void GreeterWindow::setRootImage()
{
    QPixmap pix = screen()->grabWindow(winId());
    QProcess process;
    process.start(QStandardPaths::findExecutable("lightdm-kde-greeter-rootimage"), QStringList(), QIODevice::WriteOnly);
    pix.save(&process, "xpm"); //write pixmap to rootimage
    process.closeWriteChannel();
    process.waitForFinished();
}
