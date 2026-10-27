/*
This file is part of LightDM-KDE.

Copyright (C) 2026 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QCoreApplication>
#include <QtTest>

#include "tst_Accessibility.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    TestAccessibility testAccessibility;
    return QTest::qExec(&testAccessibility, argc, argv);
}
