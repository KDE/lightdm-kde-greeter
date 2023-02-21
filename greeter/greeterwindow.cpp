/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022 Anton Golubev <golubevan@basealt.ru>

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

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickView>
#include <QScreen>
#include <QStandardPaths>
#include <QUrl>
#include <QWidget>

#include <KConfig>
#include <KConfigGroup>
#include <KQuickIconProvider>
#include <KDeclarative/KDeclarative>
#include <KLocalizedString>
#include <Plasma/Theme>

#include <QLightDM/Power>

#include "extrarowproxymodel.h"
#include "faceimageprovider.h"
#include "configwrapper.h"
#include "sessionsmodel.h"
#include "usersmodel.h"
#include "screensmodel.h"
#include "greeterwrapper.h"
#include "cursor.h"
#include "keyboard/KeyboardModel.h"

#include <config.h>

GreeterWindow::GreeterWindow(QWindow *parent)
    : QQuickView(parent)
    , m_greeter(new GreeterWrapper(this))
{
    QRect screen = QApplication::desktop()->rect();
    setGeometry(screen);

    engine()->addImageProvider(QStringLiteral("icon"), new KQuickIconProvider);

    UsersModel *usersModel = new UsersModel(this);

    if (m_greeter->hasGuestAccountHint())
    {
        usersModel->setShowGuest(true);
    }

    engine()->addImageProvider(QStringLiteral("face"), new FaceImageProvider(usersModel));

    auto keyboard = new KeyboardModel(this);

    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    KConfigGroup configGroup = config.group("greeter");
    m_enableRootImageApp = configGroup.readEntry("enable-root-image-app", "false") == QStringLiteral("true");
    QString theme = configGroup.readEntry("theme-name", "userbar");
    QUrl source { QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("themes/") + theme + QStringLiteral("/main.qml")) };
    auto cursor = new Cursor(this);

    if (source.isEmpty())
    {
        qCritical() << "Cannot find QML file for" << theme << "theme. Falling back to \"userbar\" theme.";
        theme = QStringLiteral("userbar");
        source = QUrl(QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("themes/userbar/main.qml")));
        if (source.isEmpty()) {
            qFatal("Cannot find QML file for \"userbar\" theme. Something is wrong with this installation. Aborting.");
        }
    }

    qDebug() << "Loading" << source;

    rootContext()->setContextProperty(QStringLiteral("config"), new ConfigWrapper(QStringLiteral("lightdm_theme_") + theme, this));
    rootContext()->setContextProperty(QStringLiteral("mouseCursor"), cursor);
    rootContext()->setContextProperty(QStringLiteral("screenSize"), size());
    rootContext()->setContextProperty(QStringLiteral("greeter"), m_greeter);
    rootContext()->setContextProperty(QStringLiteral("usersModel"), usersModel);
    rootContext()->setContextProperty(QStringLiteral("keyboard"), keyboard);
    rootContext()->setContextProperty(QStringLiteral("sessionsModel"), new SessionsModel(this));
    rootContext()->setContextProperty(QStringLiteral("screensModel"), new ScreensModel(this));
    rootContext()->setContextProperty(QStringLiteral("power"), new QLightDM::PowerInterface(this));
    rootContext()->setContextProperty(QStringLiteral("plasmaTheme"), new Plasma::Theme(this));

    setSource(source);

    connect(m_greeter, &GreeterWrapper::aboutToLogin, this, &GreeterWindow::setRootImage);
}

GreeterWindow::~GreeterWindow()
{
}

void GreeterWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    rootContext()->setContextProperty(QStringLiteral("screenSize"), event->size());
}

void GreeterWindow::setRootImage()
{
    if (!m_enableRootImageApp) return;
    QPixmap pix = screen()->grabWindow(winId());
    QProcess process;
    process.start(QStandardPaths::findExecutable(QStringLiteral("lightdm-kde-greeter-rootimage"), QStringList { QStringLiteral(LIBEXEC_DIR) }),
        QStringList(), QIODevice::WriteOnly);
    pix.save(&process, "xpm"); //write pixmap to rootimage
    process.closeWriteChannel();
    process.waitForFinished();
}
