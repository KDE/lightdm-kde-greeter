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

#include "extrarowproxymodel.h"

#include <QAbstractItemModel>

ExtraRowProxyModel::ExtraRowProxyModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_extraRowModel(new QStandardItemModel(this))
{
    connect(m_extraRowModel, &QStandardItemModel::rowsInserted, this, &ExtraRowProxyModel::onExtraRowsInserted);
    connect(m_extraRowModel, &QStandardItemModel::rowsRemoved, this, &ExtraRowProxyModel::onExtraRowsRemoved);
    connect(m_extraRowModel, &QStandardItemModel::dataChanged, this, &ExtraRowProxyModel::onExtraDataChanged);
}

void ExtraRowProxyModel::setSourceModel(const QSharedPointer<QAbstractItemModel> &model)
{
    if (! m_model.isNull())
    {
        disconnect(m_model.data(), &QAbstractItemModel::rowsInserted, this, &ExtraRowProxyModel::onSourceRowsInserted);
        disconnect(m_model.data(), &QAbstractItemModel::rowsRemoved, this, &ExtraRowProxyModel::onSourceRowsRemoved);
        disconnect(m_model.data(), &QAbstractItemModel::dataChanged, this, &ExtraRowProxyModel::onSourceDataChanged);
    }

    m_model = model;

    connect(m_model.data(), &QAbstractItemModel::rowsInserted, this, &ExtraRowProxyModel::onSourceRowsInserted);
    connect(m_model.data(), &QAbstractItemModel::rowsRemoved, this, &ExtraRowProxyModel::onSourceRowsRemoved);
    connect(m_model.data(), &QAbstractItemModel::dataChanged, this, &ExtraRowProxyModel::onSourceDataChanged);
}

QStandardItemModel* ExtraRowProxyModel::extraRowModel() const
{
    return m_extraRowModel;
}

QHash<int, QByteArray> ExtraRowProxyModel::roleNames() const
{
    if (!m_model.isNull())
    {
        return m_model.data()->roleNames();
    }
    else
    {
        return QAbstractListModel::roleNames();
    }
}

int ExtraRowProxyModel::rowCount(const QModelIndex &parent) const
{
    return sourceRowCount() + m_extraRowModel->rowCount();
}

QVariant ExtraRowProxyModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < sourceRowCount())
    {
        return m_model.data()->index(index.row(), 0).data(role);
    }
    else
    {
        int row = index.row() - sourceRowCount();
        return m_extraRowModel->index(row, 0).data(role);
    }
}

void ExtraRowProxyModel::onSourceRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), start, end);
    endInsertRows();
}

void ExtraRowProxyModel::onSourceRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), start, end);
    endRemoveRows();
}

void ExtraRowProxyModel::onSourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    dataChanged(createIndex(topLeft.row(), 0), createIndex(bottomRight.row(), 0));
}

void ExtraRowProxyModel::onExtraRowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginInsertRows(QModelIndex(), sourceRowCount() + start, sourceRowCount() + end);
    endInsertRows();
}

void ExtraRowProxyModel::onExtraRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    beginRemoveRows(QModelIndex(), sourceRowCount() + start, sourceRowCount() + end);
    endRemoveRows();
}

void ExtraRowProxyModel::onExtraDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    dataChanged(createIndex(sourceRowCount() + topLeft.row(), 0), createIndex(sourceRowCount() + bottomRight.row(), 0));
}

int ExtraRowProxyModel::sourceRowCount() const
{
    if (m_model.isNull())
    {
        return 0;
    }
    else
    {
        return m_model.data()->rowCount();
    }
}
