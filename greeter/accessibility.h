/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ACCESSIBILITY_H
#define ACCESSIBILITY_H

#include <QObject>

class GreeterWrapper;

class Accessibility : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool screenReaderActive READ isScreenReaderActive WRITE setScreenReaderActive NOTIFY screenReaderActiveChanged)

public:
    explicit Accessibility(GreeterWrapper *parent = nullptr);

Q_SIGNALS:

    void screenReaderActiveChanged();
    void screenReaderBackendError(const QString &errorMessage);

public Q_SLOTS:

    void handleDBusSignal(const QString &interfaceName, const QVariantMap &props, const QStringList &invalidatedProps);

private:
    bool isScreenReaderActive() { return m_screenReaderActive; }
    void setScreenReaderActive(bool active);
    void reportScreenReaderActive(bool active, bool force = false);

    void subscribeOnDBusSignals();
    void subscribeOnScreenReaderState();
    void queryScreenReaderState();
    void enableScreenReader();
    void disableScreenReader();

    bool m_screenReaderActive;
    QString m_screenReaderPreviousState;
};

#endif // ACCESSIBILITY_H
