/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "tst_GreeterWrapper.h"

#include <QtTest>
#include <QObject>
#include <QDir>
#include <QStandardPaths>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

#include "greeterwrapper.h"

using namespace Qt::StringLiterals;

static const QString s_lightdmGroup = u"lightdm"_s;
static const QString s_lastUsersKey = u"lastUser"_s;
static const QString s_lastSessionsKey = u"lastSession"_s;

TestGreeterWrapperHistory::TestGreeterWrapperHistory(QObject *parent)
    : QObject(parent)
{
}

void TestGreeterWrapperHistory::writeState(const QString &users, const QString &sessions)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig(u"state-kde"_s);
    KConfigGroup group(config, s_lightdmGroup);
    if (users.isEmpty()) {
        group.deleteEntry(s_lastUsersKey);
    } else {
        group.writeEntry(s_lastUsersKey, users);
    }
    if (sessions.isEmpty()) {
        group.deleteEntry(s_lastSessionsKey);
    } else {
        group.writeEntry(s_lastSessionsKey, sessions);
    }
    config->sync();
}

void TestGreeterWrapperHistory::initTestCase()
{
    const QString tempDir = QDir::tempPath() + u"/tst_greeterwrapper_"_s + QString::number(QCoreApplication::applicationPid());
    QVERIFY(QDir().mkpath(tempDir));
    qputenv("XDG_CONFIG_HOME", tempDir.toUtf8());
    QStandardPaths::setTestModeEnabled(true);
}

void TestGreeterWrapperHistory::cleanupTestCase()
{
    const QString tempDir = QDir::tempPath() + u"/tst_greeterwrapper_"_s + QString::number(QCoreApplication::applicationPid());
    QVERIFY(QDir(tempDir).removeRecursively());
    qunsetenv("XDG_CONFIG_HOME");
    QStandardPaths::setTestModeEnabled(false);
}

void TestGreeterWrapperHistory::init()
{
    writeState(QString(), QString());
}

void TestGreeterWrapperHistory::testEmptyHistory()
{
    GreeterWrapper greeter;
    QVERIFY(greeter.lastLoggedInUsers().isEmpty());
    QVERIFY(greeter.lastLoggedInSessions().isEmpty());
}

void TestGreeterWrapperHistory::testReadValidHistory()
{
    writeState(u"alice,bob"_s, u"plasma,plasma-wayland"_s);

    GreeterWrapper greeter;
    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"alice"_s, u"bob"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma"_s, u"plasma-wayland"_s }));
}

void TestGreeterWrapperHistory::testReadEmptySessionIsValid()
{
    writeState(u"alice,bob"_s, u"plasma,"_s);

    GreeterWrapper greeter;
    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"alice"_s, u"bob"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma"_s, QString() }));
}

void TestGreeterWrapperHistory::testReadMismatchedSizesClearsHistory()
{
    writeState(u"alice,bob"_s, u"plasma"_s);

    GreeterWrapper greeter;
    QVERIFY(greeter.lastLoggedInUsers().isEmpty());
    QVERIFY(greeter.lastLoggedInSessions().isEmpty());
}

void TestGreeterWrapperHistory::testReadEmptyUserNameClearsHistory()
{
    writeState(u"alice,,bob"_s, u"plasma,plasma,plasma"_s);

    GreeterWrapper greeter;
    QVERIFY(greeter.lastLoggedInUsers().isEmpty());
    QVERIFY(greeter.lastLoggedInSessions().isEmpty());
}

void TestGreeterWrapperHistory::testReadGuestIsNotAnError()
{
    writeState(u"*guest,alice"_s, u"plasma,plasma"_s);

    GreeterWrapper greeter;
    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"*guest"_s, u"alice"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma"_s, u"plasma"_s }));
}

void TestGreeterWrapperHistory::testReadTruncatesToLimit()
{
    writeState(u"a,b,c,d,e,f"_s, u"s1,s2,s3,s4,s5,s6"_s);

    GreeterWrapper greeter;
    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"a"_s, u"b"_s, u"c"_s, u"d"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"s1"_s, u"s2"_s, u"s3"_s, u"s4"_s }));
}

void TestGreeterWrapperHistory::testSaveNewUserGoesFirst()
{
    writeState(u"alice,bob"_s, u"plasma,plasma"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(u"charlie"_s);
    const bool ok = greeter.startSessionSync(u"plasma-wayland"_s);
    QVERIFY(ok);

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"charlie"_s, u"alice"_s, u"bob"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma-wayland"_s, u"plasma"_s, u"plasma"_s }));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(u"state-kde"_s);
    KConfigGroup group(config, s_lightdmGroup);
    QCOMPARE(group.readEntry(s_lastUsersKey), u"charlie,alice,bob"_s);
    QCOMPARE(group.readEntry(s_lastSessionsKey), u"plasma-wayland,plasma,plasma"_s);
}

void TestGreeterWrapperHistory::testSaveExistingUserMovesToFirst()
{
    writeState(u"alice,bob,carol"_s, u"s1,s2,s3"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(u"bob"_s);
    QVERIFY(greeter.startSessionSync(u"s2-new"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"bob"_s, u"alice"_s, u"carol"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"s2-new"_s, u"s1"_s, u"s3"_s }));
}

void TestGreeterWrapperHistory::testSaveRespectsLimit()
{
    writeState(u"a,b,c,d"_s, u"s1,s2,s3,s4"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(u"e"_s);
    QVERIFY(greeter.startSessionSync(u"s5"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"e"_s, u"a"_s, u"b"_s, u"c"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"s5"_s, u"s1"_s, u"s2"_s, u"s3"_s }));
}

void TestGreeterWrapperHistory::testSaveEmptyUserIsIgnored()
{
    writeState(u"alice"_s, u"plasma"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(QString());
    QVERIFY(greeter.startSessionSync(u"plasma"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"alice"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma"_s }));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(u"state-kde"_s);
    KConfigGroup group(config, s_lightdmGroup);
    QCOMPARE(group.readEntry(s_lastUsersKey), u"alice"_s);
}

void TestGreeterWrapperHistory::testSaveGuestWhenHintsEnabled()
{
    writeState(u"alice"_s, u"plasma"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(QString());
    greeter.setHasGuestAccountHint(true);
    QVERIFY(greeter.startSessionSync(u"plasma-wayland"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"*guest"_s, u"alice"_s }));
    QCOMPARE(greeter.lastLoggedInSessions(), (QStringList{ u"plasma-wayland"_s, u"plasma"_s }));

    KSharedConfig::Ptr config = KSharedConfig::openConfig(u"state-kde"_s);
    KConfigGroup group(config, s_lightdmGroup);
    QCOMPARE(group.readEntry(s_lastUsersKey), u"*guest,alice"_s);
}

void TestGreeterWrapperHistory::testSaveGuestWhenOnlyAutologinHintEnabled()
{
    GreeterWrapper greeter;
    greeter.setAuthenticationUser(QString());
    greeter.setAutologinGuestHint(true);
    QVERIFY(greeter.startSessionSync(u"plasma"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"*guest"_s }));
}

void TestGreeterWrapperHistory::testSaveEmptyUserWithoutHints()
{
    writeState(u"alice"_s, u"plasma"_s);

    GreeterWrapper greeter;
    greeter.setAuthenticationUser(QString());
    QVERIFY(greeter.startSessionSync(u"plasma"_s));

    QCOMPARE(greeter.lastLoggedInUsers(), (QStringList{ u"alice"_s }));
}

void TestGreeterWrapperHistory::testGuestLoginName()
{
    GreeterWrapper greeter;
    QCOMPARE(greeter.guestLoginName(), u"*guest"_s);
}

#include "tst_GreeterWrapper.moc"
