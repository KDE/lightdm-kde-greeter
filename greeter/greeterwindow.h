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
#ifndef GREETER_WINDOW_H
#define GREETER_WINDOW_H

#include <QQuickView>

class GreeterWrapper;

class GreeterWindow: public QQuickView
{
    Q_OBJECT

public:
    explicit GreeterWindow(QWindow *parent = nullptr);
    ~GreeterWindow();

public Q_SLOTS:
    void setRootImage();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    GreeterWrapper *m_greeter;
};

#endif // GREETER_WINDOW_H
