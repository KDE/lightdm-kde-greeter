/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2024 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QPixmap>
#include <QTimer>
#include <QGuiApplication>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#include "rootimageapp.h"

/*
Prevent a black flicker when logging in via LightDM-KDE before KSplash starts

Add a new process that when passed an image on a socket will set it to the X background
and set XSetCloseDownModel to retainTemporary.

A QML object "xhandler" exposes a slot setRootImage which will set a current screenshot of the
QML scene into the XRootWindow
This image will then be shown when switching from the greeter to the splash.
*/

RootImageApp::RootImageApp(int &argc, char **argv)
    : QApplication(argc, argv)
{
    auto x11app = nativeInterface<QNativeInterface::QX11Application>();
    Cursor arrow_cursor = XCreateFontCursor(x11app->display(), XC_left_ptr);
    XDefineCursor(x11app->display(), DefaultRootWindow(x11app->display()), arrow_cursor);

    QTimer::singleShot(0, this, SLOT(setBackground()));
}

void RootImageApp::setBackground()
{
    QFile stdIn;
    stdIn.open(0, QIODevice::ReadOnly);

    QImage image;
    image.load(&stdIn, "xpm");

    QPalette palette;
    palette.setBrush(QPalette::Window, QBrush(image));
    setPalette(palette);
    auto x11app = nativeInterface<QNativeInterface::QX11Application>();
    XClearWindow(x11app->display(), DefaultRootWindow(x11app->display()));

    quit();
}

int main(int argc, char **argv)
{
    RootImageApp app(argc, argv);
    auto x11app = app.nativeInterface<QNativeInterface::QX11Application>();
    XSetCloseDownMode(x11app->display(), RetainTemporary);

    app.exec();

    return 0;
}
