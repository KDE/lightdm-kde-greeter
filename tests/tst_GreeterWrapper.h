/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TST_GREETERWRAPPER_H
#define TST_GREETERWRAPPER_H

#include <QObject>
#include <QString>

class TestGreeterWrapperHistory : public QObject
{
    Q_OBJECT

public:
    explicit TestGreeterWrapperHistory(QObject *parent = nullptr);

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void testEmptyHistory();
    void testReadValidHistory();
    void testReadEmptySessionIsValid();
    void testReadMismatchedSizesClearsHistory();
    void testReadEmptyUserNameClearsHistory();
    void testReadGuestIsNotAnError();
    void testReadTruncatesToLimit();
    void testSaveNewUserGoesFirst();
    void testSaveExistingUserMovesToFirst();
    void testSaveRespectsLimit();
    void testSaveEmptyUserIsIgnored();
    void testSaveGuestWhenHintsEnabled();
    void testSaveGuestWhenOnlyAutologinHintEnabled();
    void testSaveEmptyUserWithoutHints();
    void testGuestLoginName();

private:
    void writeState(const QString &users, const QString &sessions);
};

#endif // TST_GREETERWRAPPER_H
