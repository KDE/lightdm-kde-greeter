/*
This file is part of LightDM-KDE.

Copyright 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2025 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SCREENSMODEL_H
#define SCREENSMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QRect>

#include <stddef.h>

class QScreen;
class QWindow;

class ScreensModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ScreensModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setFocusedWindow(QWindow *window) {
        m_focusedWindow = window;
        maintainFocusedWindow();
    };

    Q_INVOKABLE bool windowIsOnPrimaryScreen(QWindow *window);

public Q_SLOTS:
    void maintainFocusedWindow();

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
    QWindow *m_focusedWindow = nullptr;
};

#endif // SCREENSMODEL_H
