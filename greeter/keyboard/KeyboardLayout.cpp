/***************************************************************************
* Copyright (c) 2013 Nikita Mikhaylov <nslqqq@gmail.com>
*
* SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "KeyboardLayout.h"

KeyboardLayout::KeyboardLayout(QString shortName, QString longName) :
    m_short(shortName),
    m_long(longName)
{
}

QString KeyboardLayout::shortName() const
{
    return m_short;
}

QString KeyboardLayout::longName() const
{
    return m_long;
}
