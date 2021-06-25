/*
 * Button for selecting an image.
 *
 * Copyright (C) 2011  Martin Klapetek <martin.klapetek@gmail.com>
 * Copyright (C) 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2021 Aleksei Nikiforov <darktemplar@basealt.ru>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "selectimagebutton.h"

#include <QtWidgets/QWidgetAction>

#include <QImageReader>
#include <QFileDialog>
#include <QMenu>
#include <QIcon>
#include <KLocalizedString>
#include <KIconEngine>
#include <KIconLoader>

SelectImageButton::SelectImageButton(QWidget *parent)
    : QToolButton(parent)
{
    QMenu *menu = new QMenu(this);

    setPopupMode(QToolButton::InstantPopup);

    setIconSize(QSize(64,64));

    menu->addAction(QIcon(new KIconEngine(QLatin1String("document-open-folder"), KIconLoader::global())), i18n("Load from file..."), this, SLOT(onLoadImageFromFile()));
    menu->addAction(QIcon(new KIconEngine(QLatin1String("edit-clear"), KIconLoader::global())), i18n("Clear Image"), this, SLOT(onClearImage()));
    setMenu(menu);

    onClearImage();
}

SelectImageButton::~SelectImageButton()
{

}

void SelectImageButton::setImagePath(const QString &imagePath) {
    m_imagePath = imagePath;

    QPixmap image(imagePath);
    if (! image.isNull()) {
        QIcon imageIcon;
        //scale oversized avatars to fit, but don't stretch smaller images
        imageIcon.addPixmap(image.scaled(iconSize().boundedTo(image.size()), Qt::KeepAspectRatio));
        setIcon(imageIcon);
    } else {
        setIcon(QIcon(new KIconEngine(QLatin1String("image-x-generic"), KIconLoader::global())));
    }
    Q_EMIT imagePathChanged(m_imagePath);
}

QString SelectImageButton::imagePath() const {
    return m_imagePath;
}


void SelectImageButton::onLoadImageFromFile()
{
    QString fileUrl = QFileDialog::getOpenFileName(this, ki18n("Select image").toString(), QString(), QImageReader::supportedMimeTypes().join(" "), nullptr, QFileDialog::Options(QFileDialog::ReadOnly));

    if (!fileUrl.isEmpty()) {
        setImagePath(fileUrl);
    } else {
        return;
    }
}

void SelectImageButton::onClearImage()
{
    setImagePath(QString());
}
