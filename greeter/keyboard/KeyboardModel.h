/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef KEYBOARDMODEL_H
#define KEYBOARDMODEL_H

#include <QList>
#include <QObject>
#include <QString>

class KeyboardModelPrivate;
class KeyboardBackend;

class KeyboardModel : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(KeyboardModel)
public:
    // LED control
    Q_PROPERTY(bool numLock  READ numLockState  WRITE setNumLockState  NOTIFY numLockStateChanged)
    Q_PROPERTY(bool capsLock READ capsLockState WRITE setCapsLockState NOTIFY capsLockStateChanged)

    // Layouts control
    Q_PROPERTY(int currentLayout READ currentLayout WRITE setCurrentLayout NOTIFY currentLayoutChanged)
    Q_PROPERTY(QList<QObject*> layouts READ layouts NOTIFY layoutsChanged)

    Q_PROPERTY(bool enabled READ enabled CONSTANT)

public:
    KeyboardModel(QObject *parent = nullptr);
    virtual ~KeyboardModel();

Q_SIGNALS:
    void numLockStateChanged();
    void capsLockStateChanged();

    void currentLayoutChanged();
    void layoutsChanged();

public Q_SLOTS:
    bool numLockState() const;
    void setNumLockState(bool state);

    bool capsLockState() const;
    void setCapsLockState(bool state);

    QList<QObject*> layouts() const;
    int currentLayout() const;
    void setCurrentLayout(int id);

    bool enabled() const;

private Q_SLOTS:
    void dispatchEvents();

private:
    KeyboardModelPrivate * d { nullptr };
    KeyboardBackend * m_backend;
};

#endif // KEYBOARDMODEL_H
