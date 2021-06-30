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
#ifndef EXTRAROWPROXYMODEL_H
#define EXTRAROWPROXYMODEL_H

#include <QHash>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVector>
#include <QWeakPointer>

/**
 * A proxy model which makes it possible to append extra rows at the end
 */
class ExtraRowProxyModel: public QAbstractListModel
{
    Q_OBJECT

public:
    explicit ExtraRowProxyModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override; 
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setSourceModel(const QSharedPointer<QAbstractItemModel> &model);

    /** Returns a pointer to the extra row model, which can be edited as appropriate*/
    QStandardItemModel* extraRowModel() const;

    QHash<int, QByteArray> roleNames() const override;

private Q_SLOTS:
    void onSourceRowsInserted(const QModelIndex &parent, int start, int end);
    void onSourceRowsRemoved(const QModelIndex &parent, int start, int end);
    void onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void onExtraRowsInserted(const QModelIndex &parent, int start, int end);
    void onExtraRowsRemoved(const QModelIndex &parent, int start, int end);
    void onExtraDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private:
    typedef QHash<int, QVariant> Item; //role, item.
    typedef QHash<int, Item> Row; //column, item
    typedef QVector<Row> Rows;

    int sourceRowCount() const;

    QSharedPointer<QAbstractItemModel> m_model;
    QStandardItemModel *m_extraRowModel;
    Rows m_rows;
};

#endif /* EXTRAROWPROXYMODEL_H */
