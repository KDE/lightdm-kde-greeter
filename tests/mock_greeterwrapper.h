/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QObject>
#include <QMap>

#ifndef MOCK_GREETERWRAPPER_H
#define MOCK_GREETERWRAPPER_H

class GreeterWrapper : public QObject {

    Q_OBJECT

public:
    QString getAccessibilitySetting(const QString &key) const
    {
        return m_accessibilitySettings.value(key);
    }

    void setAccessibilitySetting(const QString &key, const QString &value)
    {
        m_accessibilitySettings[key] = value;
    }

private:
    QMap<QString, QString> m_accessibilitySettings;
};

#endif // MOCK_GREETERWRAPPER_H
