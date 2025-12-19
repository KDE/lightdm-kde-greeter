/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "screensmodel.h"

#include <QApplication>
#include <QScreen>
#include <QWindow>

ScreensModel::ScreensModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(qGuiApp, &QApplication::screenAdded,          this, [this] (QScreen */*screen*/) { loadScreens(); });
    connect(qGuiApp, &QApplication::screenRemoved,        this, [this] (QScreen */*screen*/) { loadScreens(); });
    connect(qGuiApp, &QApplication::primaryScreenChanged, this, [this] (QScreen */*screen*/) { loadScreens(); });

    m_roles[Qt::UserRole] = "geometry";

    loadScreens();
}

QHash<int, QByteArray> ScreensModel::roleNames() const
{
    return m_roles;
}

int ScreensModel::rowCount(const QModelIndex &parent) const
{
    if (parent == QModelIndex())
    {
        return m_screens.size();
    }

    return 0;
}

QVariant ScreensModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row < 0 || row >= m_screens.size())
    {
        return QVariant();
    }

    if (role == Qt::UserRole)
    {
        return m_screens[row].geometry;
    }

    return QVariant();
}

void ScreensModel::onScreenResized(size_t screen, const QRect &geometry)
{
    if (screen < size_t(m_screens.size()))
    {
        m_screens[screen].geometry = geometry;
    }

    QModelIndex index = createIndex(screen,0);
    dataChanged(index, index);
}

void ScreensModel::loadScreens()
{
    beginResetModel();

    for (const auto &screen_data: m_screens)
    {
        disconnect(screen_data.ptr, &QScreen::geometryChanged, this, nullptr);
    }

    m_screens.clear();

    size_t screen_number = 0;
    auto screens_list = qGuiApp->screens();

    for (auto screen = screens_list.constBegin(); screen != screens_list.constEnd(); ++screen, ++screen_number)
    {
        connect(*screen, &QScreen::geometryChanged, this, [this, screen_number] (const QRect &geometry) { onScreenResized(screen_number, geometry); });
        m_screens.append(ScreenData{*screen, (*screen)->geometry()});
    }

    endResetModel();
}

void ScreensModel::maintainFocusedWindow()
{
    auto current = QGuiApplication::focusWindow();
    if (m_focusedWindow && current != m_focusedWindow) {
        m_focusedWindow->requestActivate();
    }
}

bool ScreensModel::windowIsOnPrimaryScreen(QWindow *window)
{
    if (!window) return false;
    auto screen = window->screen();
    if (!screen) return false;
    return screen == QGuiApplication::primaryScreen();
}
