/*
This file is part of LightDM-KDE.

Copyright (C) 2023 Anton Golubev <golubevan@altlinux.org>

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

#ifndef CURSOR_H
#define CURSOR_H

#include <QObject>
#include <QCursor>

class Cursor : public QObject
{
    Q_OBJECT

public:
    explicit Cursor(QObject *parent = nullptr) : QObject(parent) {}

public Q_SLOTS:
    void move(int x, int y) { QCursor::setPos(x, y); }
};

#endif // CURSOR_H
