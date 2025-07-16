/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023-2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QCommandLineParser>
#include <QApplication>
#include <QQuickStyle>
#include <QDBusInterface>

#include <KAboutData>
#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

#include "../about.h"
#include "greeterwindow.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <cmath>

struct MyScreen {
    QString name;
    int width;
    int height;
    int mm_width;
    int mm_height;
    operator QString() const { return QStringLiteral("output %1 width %2 height %3 mm_width %4 mm_height %5").arg(name).arg(width).arg(height).arg(mm_width).arg(mm_height); }
};

QList<MyScreen> getScreens(float &dpi) {
    QList<MyScreen> result;
    Display *dpy = XOpenDisplay(nullptr);

    if (!dpy) {
        qWarning("%s: can't open X display", __FUNCTION__);
        return result;
    }

    dpi = 0.f;

    do {

        int screen_count = ScreenCount(dpy);
        if (screen_count != 1) {
            qWarning("%s: don't know what to do if X-screens are not exactly 1 (their %d)", __FUNCTION__, screen_count);
            break;
        }

        auto sod = ScreenOfDisplay(dpy, 0);
        dpi = 25.4f * sod->width / sod->mwidth;
        qDebug("%s: X-screen info: width %d height %d widthmm %d heightmm %d (dpi %.3f)",
                __FUNCTION__,
                sod->width, sod->height, sod->mwidth, sod->mheight, dpi);

        int event_base, error_base;
        int major, minor;
        if (!XRRQueryExtension (dpy, &event_base, &error_base) || !XRRQueryVersion (dpy, &major, &minor)) {
            qWarning("%s: RandR extension missing", __FUNCTION__);
            break;
        }

        qDebug("%s: using RandR v%d.%d", __FUNCTION__, major, minor);
        if (!(major > 1 || (major == 1 && minor >= 5))) {
            qWarning("%s: need RandR > v1.5", __FUNCTION__);
            break;
        }

        int n;
        int get_active = 0;
        Window root = RootWindow(dpy, 0);
        XRRMonitorInfo *m = XRRGetMonitors(dpy, root, get_active, &n);
        if (n == -1) {
            qWarning("%s: Get monitors failed", __FUNCTION__);
            break;
        }

        XRRScreenResources *res = XRRGetScreenResources(dpy, root);
        for (int i = 0; i < n; ++i) {
            if (m[i].noutput != 1) {
                qWarning("%s: (monitor %d of %d) I don't know what to do if there is not exactly one output for the monitor", __FUNCTION__, i + 1, n);
                continue;
            }
            XRROutputInfo *output = XRRGetOutputInfo(dpy, res, m[i].outputs[0]);
            result.push_back({ QLatin1String(output->name), m[i].width, m[i].height, m[i].mwidth, m[i].mheight });
            qDebug("%s: added screen %s", __FUNCTION__, qPrintable(result.back()));
            XRRFreeOutputInfo(output);
        }
        XRRFreeScreenResources(res);

    } while(false);

    XCloseDisplay(dpy);
    return result;
}

// pass a smaller screen size here
float scaleForScreen(float pSize, float ppi, float dpi)
{
    qDebug("%s: scaling screen pSize %.3f ppi %.3f dpi %.3f", __FUNCTION__, pSize, ppi, dpi);
    const float baseSize = 1080.f;
    float ratio = pSize / baseSize;

    const float step = 0.5f;
    ratio = std::round(ratio / step) * step;
    ratio = std::min(ratio, 4.0f);
    ratio = std::max(ratio, 1.0f);

    return ratio;
}

void applyScreenScales()
{
    auto scaleFactors = qgetenv("QT_SCREEN_SCALE_FACTORS");
    if (scaleFactors.length() > 0) {
        qDebug("%s: someone set the values manually (%s) - let's not mess them up", __FUNCTION__, scaleFactors.data());
        return;
    }

    float dpi = 0.f;
    auto screens = getScreens(dpi);
    if (screens.size() == 0) return;

    QString sep, param;
    for(auto &screen: screens) {
        float ppi = 25.4f * (float)screen.width / screen.mm_width;
        float scale = scaleForScreen(std::min(screen.height, screen.width), ppi, dpi);
        // do not write the value 1, this can cause scale inconsistencies in rare cases
        if (scale == 1.f) continue;
        param += QStringLiteral("%1%2=%3").arg(sep).arg(screen.name).arg(scale);
        sep = QStringLiteral(";");
    }

    qDebug("%s: Apply scale factors: '%s'", __FUNCTION__, param.toLocal8Bit().data());
    qputenv("QT_SCREEN_SCALE_FACTORS", param.toUtf8());
}

int main(int argc, char **argv)
{
    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    KConfigGroup configGroup = config.group(QStringLiteral("greeter"));

    qputenv("QT_IM_MODULE", configGroup.readEntry("input-method", "qtvirtualkeyboard").toUtf8());

    // find out if we are on Wayland
    // QGuiApplication::platformName() is not yet defined properly
    // how it is done in QGuiApplicationPrivate::createPlatformIntegration
    const bool hasWaylandDisplay = qEnvironmentVariableIsSet("WAYLAND_DISPLAY");
    const bool isWaylandSessionType = qgetenv("XDG_SESSION_TYPE") == "wayland";

    // need to do some Xorg-specific adjustmenst, before declaring the Qt application
    if (!hasWaylandDisplay && !isWaylandSessionType) {
        applyScreenScales();
    }

    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

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

    QDBusInterface qiface(QStringLiteral("org.freedesktop.systemd1"),
                          QStringLiteral("/org/freedesktop/systemd1"),
                          QStringLiteral("org.freedesktop.systemd1.Manager"),
                          QDBusConnection::sessionBus());

    qiface.asyncCall(QStringLiteral("RestartUnit"),
                                   QStringLiteral("lightdm-kde-greeter-wifikeeper.service"),
                                   QStringLiteral("replace"));

    GreeterWindow *w = new GreeterWindow();
    w->show();

    // receive focus
    if (QGuiApplication::primaryScreen() == w->screen()) {
        w->requestActivate();
    }

    return app.exec();
}
