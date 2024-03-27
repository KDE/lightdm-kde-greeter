/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SCREENSMODEL_H
#define SCREENSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QRect>

#include <stddef.h>

class QScreen;

class ScreensModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ScreensModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
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
