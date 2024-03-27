/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef FACEIMAGEPROVIDER_H
#define FACEIMAGEPROVIDER_H

#include <QQuickImageProvider>

namespace QLightDM
{
    class UsersModel;
}

class QAbstractItemModel;

class FaceImageProvider: public QQuickImageProvider
{
public:
    explicit FaceImageProvider(QAbstractItemModel *model);

    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QAbstractItemModel *m_model;
};

#endif
