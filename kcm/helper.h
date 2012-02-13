/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>

LightDM-KDE is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

LightDM-KDE is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with LightDM-KDE.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef HELPER_H
#define HELPER_H

#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>

using namespace KAuth;

class Helper: public QObject
{
    Q_OBJECT
public slots:
    ActionReply savetheme(const QVariantMap &args);
    ActionReply savethemedetails(const QVariantMap &args);
};

#endif
