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

#ifndef SELECTIMAGEBUTTONDESIGNERFACTORY_H
#define SELECTIMAGEBUTTONDESIGNERFACTORY_H

#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QObject>

#define QT_STATICPLUGIN

class SelectImageButtonDesignerFactory : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.alt.lightdm_config_widgets" FILE "lightdm_config_widgets.json")
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit SelectImageButtonDesignerFactory(QObject *parent = nullptr);

    bool isContainer() const override;
    QIcon icon() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget* createWidget(QWidget *parent) override;
};



#endif // SELECTIMAGEBUTTONDESIGNERFACTORY_H
