/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QCoreApplication>
#include <QtTest>

#include "tst_Accessibility.h"
#include "tst_GreeterWrapper.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    int result;

    TestAccessibility testAccessibility;
    result = QTest::qExec(&testAccessibility, argc, argv);

    if (result != 0) return result;

    TestGreeterWrapperHistory testGreeterWrapper;
    result = QTest::qExec(&testGreeterWrapper, argc, argv);

    return result;
}
