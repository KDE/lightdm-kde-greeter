/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
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
