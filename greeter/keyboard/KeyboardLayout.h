/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef KEYBOARDLAYOUT_H
#define KEYBOARDLAYOUT_H

#include <QtCore/QObject>

class KeyboardLayout : public QObject {
Q_OBJECT
    Q_PROPERTY(QString shortName READ shortName CONSTANT)
    Q_PROPERTY(QString longName READ longName CONSTANT)
public:
    KeyboardLayout(QString shortName, QString longName);

    virtual ~KeyboardLayout() = default;

    QString shortName() const;
    QString longName() const;

private:
    QString m_short, m_long;
};

#endif // KEYBOARDLAYOUT_H
