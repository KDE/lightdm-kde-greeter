/*
This file is part of LightDM-KDE.

Copyright 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>

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

#include "selectimagebuttondesignerfactory.h"

#include <QtCore/QtPlugin>

#include "selectimagebutton.h"

SelectImageButtonDesignerFactory::SelectImageButtonDesignerFactory(QObject *parent)
    : QObject(parent)
{
}

bool SelectImageButtonDesignerFactory::isContainer() const
{
    return false;
}

QIcon SelectImageButtonDesignerFactory::icon() const
{
    return QIcon();
}

QString SelectImageButtonDesignerFactory::group() const
{
    return QString();
}

QString SelectImageButtonDesignerFactory::includeFile() const
{
    return QStringLiteral("selectimagebutton.h");
}

QString SelectImageButtonDesignerFactory::name() const
{
    return QStringLiteral("SelectImageButton");
}

QString SelectImageButtonDesignerFactory::toolTip() const
{
    return QString();
}

QString SelectImageButtonDesignerFactory::whatsThis() const
{
    return QString();
}

QWidget* SelectImageButtonDesignerFactory::createWidget(QWidget *parent)
{
    return new SelectImageButton(parent);
}





