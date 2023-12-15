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

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <cmath>

struct MyScreen {
    int width;
    int height;
    int mm_width;
    int mm_height;
    operator QString() const { return QStringLiteral("width %1 height %2 mm_width %3 mm_height %4").arg(width).arg(height).arg(mm_width).arg(mm_height); }
};

QList<MyScreen> getScreens(float &dpi) {
    QList<MyScreen> result;
    Display *dpy = XOpenDisplay(nullptr);
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

        for (int i = 0; i < n; ++i) {
            result.push_back({ m[i].width, m[i].height, m[i].mwidth, m[i].mheight });
        }

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

    QString sep, param, empty;
    for(auto &screen: screens) {
        float ppi = 25.4f * (float)screen.width / screen.mm_width;
        float scale = scaleForScreen(std::min(screen.height, screen.width), ppi, dpi);
        // do not write the value 1, this can cause scale inconsistencies in rare cases
        param += sep + (scale == 1.f ? empty : QString::number(scale));
        sep = QStringLiteral(";");
    }

    qDebug("%s: Apply scale factors: '%s'", __FUNCTION__, param.toLocal8Bit().data());
    qputenv("QT_SCREEN_SCALE_FACTORS", param.toUtf8());
}

int main(int argc, char **argv)
{
    KConfig config(QStringLiteral(LIGHTDM_CONFIG_DIR "/lightdm-kde-greeter.conf"));
    KConfigGroup configGroup = config.group("greeter");

    qputenv("QT_IM_MODULE", configGroup.readEntry("input-method", "qtvirtualkeyboard").toUtf8());

    // need to do this before declaring the Qt application
    applyScreenScales();

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
