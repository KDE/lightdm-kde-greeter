/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
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

#include "screensmodel.h"

#include <QApplication>
#include <QScreen>

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
    if (screen < m_screens.size())
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
