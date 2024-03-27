/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef HELPER_H
#define HELPER_H

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

class Helper: public QObject
{
    Q_OBJECT

public Q_SLOTS:
    KAuth::ActionReply save(const QVariantMap &args);

private:
    QString copyImage(QString sourceFile, QString theme, QString name);
};

#endif
