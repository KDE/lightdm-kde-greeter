/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MOCK_ACCESSIBILITYBACKEND_H
#define MOCK_ACCESSIBILITYBACKEND_H

#include <QObject>

class MockBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool screenReaderActive READ screenReaderActive WRITE setScreenReaderActive NOTIFY screenReaderActiveChanged)

public:
    bool screenReaderActive() const { return m_screenReaderActive; }

    // The property isn’t set until a signal arrives from the service.
    void setScreenReaderActive(bool) {}

    Q_INVOKABLE void actuallySetScreenReaderActive(bool v, bool emit = true) {
        m_screenReaderActive = v;
        if (emit) Q_EMIT screenReaderActiveChanged();
    }

    Q_INVOKABLE void emitError(const QString &msg) {
        Q_EMIT screenReaderBackendError(msg);
    }

Q_SIGNALS:
    void screenReaderActiveChanged();
    void screenReaderBackendError(const QString &errorMessage);

private:
    bool m_screenReaderActive = false;
};

#endif // MOCK_ACCESSIBILITYBACKEND_H
