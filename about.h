/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef ABOUT_H
#define ABOUT_H

#include <config.h>

// Helper function to factorize common part s of KAboutData definition between
// greeter and kcm
static void initAboutData(KAboutData *aboutData)
{
    aboutData->setVersion(LIGHTDM_KDE_VERSION);

    aboutData->setShortDescription(ki18n("Login screen using the LightDM framework").toString());
    aboutData->setLicense(KAboutLicense::GPL, KAboutLicense::OrLaterVersions);
    aboutData->setCopyrightStatement(ki18n("(c) 2012 David Edmundson").toString());
    aboutData->setHomepage(QStringLiteral("http://git.altlinux.org/gears/l/lightdm-kde-greeter.git"));

    aboutData->addAuthor(ki18n("David Edmundson").toString(), ki18n("Author").toString(), QStringLiteral("kde@davidedmundson.co.uk"));
    aboutData->addAuthor(ki18n("Aurélien Gâteau").toString(), ki18n("Developer").toString(), QStringLiteral("aurelien.gateau@canonical.com"));
    aboutData->addAuthor(ki18n("Aleksei Nikiforov").toString(), ki18n("Developer").toString(), QStringLiteral("darktemplar@basealt.ru"));
    aboutData->addAuthor(ki18n("Anton Golubev").toString(), ki18n("Developer").toString(), QStringLiteral("golubevan@altlinux.org"));
}

#endif /* ABOUT_H */
