/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QPixmap>
#include <QTimer>
#include <QX11Info>

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
    Cursor arrow_cursor = XCreateFontCursor(QX11Info::display(), XC_left_ptr);
    XDefineCursor(QX11Info::display(), QX11Info::appRootWindow(), arrow_cursor);

    QTimer::singleShot(0, this, SLOT(setBackground()));
}

void RootImageApp::setBackground()
{
    QFile stdIn;
    stdIn.open(0, QIODevice::ReadOnly);

    QImage image;
    image.load(&stdIn, "xpm");

    QPalette palette;
    palette.setBrush(desktop()->backgroundRole(), QBrush(image));
    desktop()->setPalette(palette);
    XClearWindow(QX11Info::display(), desktop()->winId());

    quit();
}

int main(int argc, char **argv)
{
    RootImageApp app(argc, argv);
    XSetCloseDownMode(QX11Info::display(), RetainTemporary);

    app.exec();

    return 0;
}
