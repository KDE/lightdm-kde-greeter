/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef THEMESMODEL_H
#define THEMESMODEL_H

#include <QAbstractListModel>

class ThemeItem;
class QDir;

class ThemesModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole,
        AuthorRole,
        DescriptionRole,
        VersionRole,
        PreviewRole,
        PathRole
    };

    explicit ThemesModel(QObject *parent = nullptr);

    Q_INVOKABLE int indexForId(QString id) const;
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

private:
    void load();
    void loadTheme(const QDir &themePath);
    QList<ThemeItem*> m_themes;
};

#endif // THEMESMODEL_H
