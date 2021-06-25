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

#ifndef SCREENSMODEL_H
#define SCREENSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QRect>

#include <stddef.h>

class QScreen;

class ScreensModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ScreensModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;

    QHash<int, QByteArray> roleNames() const override;

private slots:
    void onScreenResized(size_t screen, const QRect &geometry);

private:
    void loadScreens();

    struct ScreenData
    {
        QScreen *ptr;
        QRect geometry;
    };

    QList<ScreenData> m_screens;

    QHash<int, QByteArray> m_roles;
};

#endif // SCREENSMODEL_H
