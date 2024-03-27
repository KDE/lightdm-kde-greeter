/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "faceimageprovider.h"

#include <QLightDM/UsersModel>

#include <QDebug>
#include <QFile>
#include <QIcon>
#include <QPixmap>

#include <KIconLoader>
#include <KIconEngine>

FaceImageProvider::FaceImageProvider(QAbstractItemModel *model)
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , m_model(model)
{
}

QPixmap FaceImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    // Lookup user in model
    QModelIndex userIndex;

    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row)
    {
        QModelIndex index = m_model->index(row, 0);
        if (index.data(QLightDM::UsersModel::NameRole).toString() == id)
        {
            userIndex = index;
            break;
        }
    }

    if (!userIndex.isValid())
    {
        qWarning() << "Couldn't find user" << id << "in UsersModel";
        return QPixmap();
    }

    // Get user face pixmap
    QPixmap pix;
    int extent = requestedSize.isValid() ? requestedSize.width() : KIconLoader::SizeEnormous;
    QString imagePath = userIndex.data(QLightDM::UsersModel::ImagePathRole).toString();

    //Work around a bug in AccountsService which returns the image path as "~/.face" regardless of whether it exists or not.
    //we check if it exists, and also search "~/.face.icon"

    //if there is an image path, and the image actually exists
    if (!imagePath.isNull())
    {
        pix.load(imagePath + QStringLiteral(".icon"));
        if (pix.isNull())
        {
            pix.load(imagePath);
        }
    }

    if (size)
    {
        *size = pix.size();
    }

    if (requestedSize.isValid())
    {
        pix = pix.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return pix;
}
