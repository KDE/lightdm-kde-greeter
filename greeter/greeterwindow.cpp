/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2022-2024 Anton Golubev <golubevan@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "greeterwindow.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QHostInfo>
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
#include <KLocalizedString>
#include <Plasma/Theme>

#include <QLightDM/Power>
#include <QLightDM/UsersModel>

#include "extrarowproxymodel.h"
#include "faceimageprovider.h"
#include "configwrapper.h"
#include "sessionsmodel.h"
#include "usersmodel.h"
#include "connectionsmodel.h"
#include "screensmodel.h"
#include "greeterwrapper.h"
#include "cursor.h"
#include "keyboard/KeyboardModel.h"

#include <config.h>

GreeterWindow::GreeterWindow(QWindow *parent)
    : QQuickView(parent)
    , m_greeter(new GreeterWrapper(this))
{
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    setGeometry(screen);

    engine()->addImageProvider(QStringLiteral("icon"), new KQuickIconProvider);

    UsersModel *usersModel = new UsersModel(this);

    if (m_greeter->hasGuestAccountHint())
    {
        usersModel->setShowGuest(true);
    }

    // add the last logged in user to the model, if it is not present in the model (perhaps this is a domain user).
    QString lastUserName = m_greeter->lastLoggedInUser();
    if (lastUserName.length() > 0 && usersModel->indexForUserName(lastUserName) < 0) {
        QStandardItem *lastUser = new QStandardItem(lastUserName);
        lastUser->setData(lastUserName, QLightDM::UsersModel::NameRole);
        lastUser->setData(m_greeter->lastLoggedInSession(), QLightDM::UsersModel::SessionRole);
        lastUser->setData(QStringLiteral("/home/%1/.face").arg(lastUserName), QLightDM::UsersModel::ImagePathRole);
        usersModel->extraRowModel()->appendRow(lastUser);
    }

    qmlRegisterUncreatableMetaObject(ConnectionEnum::staticMetaObject, "ConnectionEnum", 1, 0, "ConnectionEnum", QStringLiteral("Error: only enums"));
    qRegisterMetaType<ConnectionEnum::Action>();

    engine()->addImageProvider(QStringLiteral("face"), new FaceImageProvider(usersModel));

    auto keyboard = new KeyboardModel(this);

    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    KConfigGroup configGroup = config.group(QStringLiteral("greeter"));
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
    rootContext()->setContextProperty(QStringLiteral("generalConfig"), new ConfigWrapper(QStringLiteral("greeter"), this));
    rootContext()->setContextProperty(QStringLiteral("mouseCursor"), cursor);
    rootContext()->setContextProperty(QStringLiteral("screenSize"), size());
    rootContext()->setContextProperty(QStringLiteral("greeter"), m_greeter);
    rootContext()->setContextProperty(QStringLiteral("usersModel"), usersModel);
    rootContext()->setContextProperty(QStringLiteral("connectionsModel"), new ConnectionsModel(this));
    rootContext()->setContextProperty(QStringLiteral("keyboard"), keyboard);
    rootContext()->setContextProperty(QStringLiteral("sessionsModel"), new SessionsModel(this));
    rootContext()->setContextProperty(QStringLiteral("screensModel"), new ScreensModel(this));
    rootContext()->setContextProperty(QStringLiteral("power"), new QLightDM::PowerInterface(this));
    rootContext()->setContextProperty(QStringLiteral("plasmaTheme"), new Plasma::Theme(this));
    rootContext()->setContextProperty(QStringLiteral("defaultWallpaper"), QStringLiteral(GREETER_DEFAULT_WALLPAPER));
    rootContext()->setContextProperty(QStringLiteral("localHostName"), QHostInfo::localHostName());

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
