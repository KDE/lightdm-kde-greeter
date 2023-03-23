/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
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
#include "themesmodel.h"

#include <QDebug>
#include <QDir>
#include <QPixmap>
#include <QList>
#include <QStandardPaths>
#include <QSettings>
#include <QString>

#include <KConfigGroup>
#include <KDesktopFile>

class ThemeItem
{
public:
    QString id;
    QString name;
    QString description;
    QString author;
    QString version;
    QPixmap preview;
    QString path;
};

ThemesModel::ThemesModel(QObject *parent)
    : QAbstractListModel(parent)
{
    //FUTURE FIXME: do the single shot trick so we can start displaying the UI
    //before bothering to do the loading.
    //will need emit on finished.
    this->load();
}

int ThemesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_themes.size();
}

QVariant ThemesModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    switch (role)
    {
    case Qt::DisplayRole:
        return m_themes[row]->name;
    case Qt::DecorationRole:
        if (m_themes[row]->preview.isNull())
        {
            return QVariant();
        }
        //FIXME shouldn't really be scaling here, it's a bit slow - in the delegate is better.
        return m_themes[row]->preview.scaled(QSize(100,100), Qt::KeepAspectRatio);
    case ThemesModel::PreviewRole:
        return m_themes[row]->preview;
    case ThemesModel::IdRole:
        return m_themes[row]->id;
    case ThemesModel::DescriptionRole:
        return m_themes[row]->description;
    case ThemesModel::VersionRole:
        return m_themes[row]->version;
    case ThemesModel::AuthorRole:
        return m_themes[row]->author;
    case ThemesModel::PathRole:
        return m_themes[row]->path;
    }

    return QVariant();
}

void ThemesModel::load()
{
    qDebug() << "loading themes";
    QStringList themeDirPaths = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("lightdm-kde-greeter/themes"), QStandardPaths::LocateDirectory);
    qDebug() << themeDirPaths;

    //get a list of possible theme directories, loop through each of these finding themes.
    //FIXME I think this can be simplified to return all possible themes directly

    for (const auto &themeDirPath: themeDirPaths)
    {
        QDir themeDir(themeDirPath);
        foreach (const QString &dirPath, themeDir.entryList(QDir::NoDotAndDotDot | QDir::Dirs))
        {
            qDebug() << themeDir.filePath(dirPath + QStringLiteral("/theme.desktop"));
            if (QFile::exists(themeDir.filePath(dirPath + QStringLiteral("/theme.desktop"))))
            {
                loadTheme(QDir(themeDir.filePath(dirPath)));
            }
        }
    }
}

void ThemesModel::loadTheme(const QDir &themePath)
{
    KDesktopFile themeInfo(themePath.filePath(QStringLiteral("theme.desktop")));

    ThemeItem *theme = new ThemeItem;
    theme->id = themePath.dirName();
    theme->name = themeInfo.readName();
    theme->description = themeInfo.readComment();
    theme->author = themeInfo.desktopGroup().readEntry("author");
    theme->version = themeInfo.desktopGroup().readEntry("version");

    theme->preview = QPixmap(themePath.absoluteFilePath(QStringLiteral("preview.png")));
    theme->path = themePath.path();

    qDebug() << "adding theme" << theme->name;

    beginInsertRows(QModelIndex(), m_themes.size(), m_themes.size()+1);
    m_themes.append(theme);
    endInsertRows();
}

QHash<int,QByteArray> ThemesModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "name";
    roles[PreviewRole]     = "preview";
    roles[IdRole]          = "id";
    roles[DescriptionRole] = "description";
    roles[VersionRole]     = "version";
    roles[AuthorRole]      = "author";
    roles[PathRole]        = "path";
    return roles;
}

int ThemesModel::indexForId(QString id) const
{
    int i = 0;
    while (hasIndex(i, 0)) {
        if (data(index(i), IdRole) == id) {
            return i;
        }
        ++i;
    }
    return 0;
}
