/* $BEGIN_LICENSE

This file is part of Musique.
Copyright 2013, Flavio Tordini <flavio.tordini@gmail.com>

Musique is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Musique is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Musique.  If not, see <http://www.gnu.org/licenses/>.

$END_LICENSE */

#include "finderitemdelegate.h"
#include "basesqlmodel.h"
#include "finderwidget.h"
#include "model/artist.h"
#include "model/album.h"
#include "model/folder.h"
#include "fontutils.h"
#include "iconutils.h"

const int FinderItemDelegate::ITEM_HEIGHT = 150;
const int FinderItemDelegate::ITEM_WIDTH = 150;
const int FinderItemDelegate::PADDING = 10;

FinderItemDelegate::FinderItemDelegate(QObject* parent) : QStyledItemDelegate(parent) { }

QPixmap FinderItemDelegate::createPlayIcon(bool hovered, qreal pixelRatio) {
    const int iconHeight = 24;
    const int iconWidth = 24;
    const int PADDING = 4;

    QPixmap playIcon = QPixmap(iconWidth*2*pixelRatio, iconHeight*2*pixelRatio);
    playIcon.setDevicePixelRatio(pixelRatio);
    playIcon.fill(Qt::transparent);
    QPainter painter(&playIcon);
    painter.setRenderHints(QPainter::Antialiasing, true);

    QColor black = Qt::black;
    QColor white = Qt::white;

    if (hovered) {
        black = Qt::white;
        white = Qt::black;
    }
    painter.setBrush(black);
    QPen pen(white);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawEllipse(QPoint(iconWidth, iconHeight), iconWidth-2, iconHeight-2);

    painter.translate(iconWidth/2, iconHeight/2);
    QPolygon polygon;
    polygon << QPoint(PADDING*2, PADDING)
            << QPoint(iconWidth - PADDING, iconHeight/2)
            << QPoint(PADDING*2, iconHeight - PADDING);
    painter.setBrush(white);
    pen.setColor(white);
    pen.setWidth(3);
    pen.setJoinStyle(Qt::RoundJoin);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawPolygon(polygon);

    return playIcon;
}

QPixmap FinderItemDelegate::createMissingItemBackground(qreal pixelRatio) {
    const int w = ITEM_WIDTH * pixelRatio;
    const int h = ITEM_HEIGHT * pixelRatio;

    QPixmap pixmap = QPixmap(w, h);

    QPainter painter(&pixmap);

    QRadialGradient radialGrad(QPoint(w/2, h/3), w);
    radialGrad.setColorAt(0, QColor(48, 48, 48));
    radialGrad.setColorAt(1, Qt::black);
    painter.setBrush(radialGrad);
    painter.setPen(Qt::NoPen);
    painter.drawRect(QRect(0, 0, w, h));

    pixmap.setDevicePixelRatio(pixelRatio);
    return pixmap;
}

const QPixmap &FinderItemDelegate::getMissingItemPixmap(const QString &type) {
    static QHash<QString, QPixmap> cache;
    const qreal pixelRatio = IconUtils::pixelRatio();
    const QString key = type + QString::number(pixelRatio);
    QHash<QString, QPixmap>::const_iterator i = cache.constFind(key);
    if (i != cache.constEnd()) return i.value();

    QPixmap pixmap = QPixmap(getMissingItemBackground(pixelRatio));
    pixmap.setDevicePixelRatio(pixelRatio);
    QPainter painter(&pixmap);

    QPixmap symbol = IconUtils::pixmap(":/images/item/" + type + ".png");
    painter.setOpacity(.1);
    painter.drawPixmap(((ITEM_WIDTH - symbol.width()) / 2) * pixelRatio,
                       ((ITEM_HEIGHT - symbol.height()) / 3) * pixelRatio, symbol);

    return cache.insert(key, pixmap).value();
}

const QPixmap &FinderItemDelegate::getPlayIcon(bool hovered) {
    static QHash<QString, QPixmap> cache;
    const qreal pixelRatio = IconUtils::pixelRatio();
    const QString key = (hovered ? "1|" : "0|") + QString::number(pixelRatio);
    QHash<QString, QPixmap>::const_iterator i = cache.constFind(key);
    if (i != cache.constEnd()) return i.value();
    QPixmap pixmap = createPlayIcon(hovered, pixelRatio);
    return cache.insert(key, pixmap).value();
}

const QPixmap &FinderItemDelegate::getMissingItemBackground(qreal pixelRatio) {
    static QHash<QString, QPixmap> cache;
    const QString key = QString::number(pixelRatio);
    QHash<QString, QPixmap>::const_iterator i = cache.constFind(key);
    if (i != cache.constEnd()) return i.value();
    QPixmap pixmap = createMissingItemBackground(pixelRatio);
    return cache.insert(key, pixmap).value();
}

const QPixmap &FinderItemDelegate::getMissingArtistPixmap() {
    return getMissingItemPixmap("artist");
}

const QPixmap &FinderItemDelegate::getMissingAlbumPixmap() {
    return getMissingItemPixmap("album");
}

const QPixmap &FinderItemDelegate::getMissingTrackPixmap() {
    return getMissingItemPixmap("track");
}

QSize FinderItemDelegate::sizeHint( const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/ ) const {
    return QSize(ITEM_WIDTH, ITEM_HEIGHT);
}

void FinderItemDelegate::paint( QPainter* painter,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index ) const {

    int itemType = index.data(Finder::ItemTypeRole).toInt();

    if (itemType == Finder::ItemTypeArtist) {
        paintArtist(painter, option, index);
    } else if (itemType == Finder::ItemTypeAlbum) {
        paintAlbum(painter, option, index);
    } else if (itemType == Finder::ItemTypeFolder) {
        paintFolder(painter, option, index);
    } else if (itemType == Finder::ItemTypeTrack) {
        paintTrack(painter, option, index);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }

}

void FinderItemDelegate::paintArtist(QPainter* painter,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {

    // get the data object
    const ArtistPointer artistPointer = index.data(Finder::DataObjectRole).value<ArtistPointer>();
    Artist *artist = artistPointer.data();
    if (!artist) return;

    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    const bool isHovered = index.data(Finder::HoveredItemRole ).toBool();
    const bool isSelected = option.state & QStyle::State_Selected;

    // thumb
    QPixmap pixmap = getArtistPixmap(artist);
    if (pixmap.isNull()) pixmap = getMissingArtistPixmap();
    painter->drawPixmap(0, 0, pixmap);

    // play icon overlayed on the thumb
    if (isHovered) {
        double animation = index.data(Finder::PlayIconAnimationItemRole).toDouble();
        bool playIconHovered = index.data(Finder::PlayIconHoveredRole).toBool();
        paintPlayIcon(painter, line, animation, playIconHovered);
    }

    // name
    drawName(painter, option, artist->getName(), line, isSelected);

    // if (artist->getTrackCount() > 0) {
    // drawBadge(painter, QString::number(artist->getTrackCount()), line);
    // }

    painter->restore();

}

void FinderItemDelegate::paintAlbum(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {

    // get the data object
    const AlbumPointer albumPointer = index.data(Finder::DataObjectRole).value<AlbumPointer>();
    Album *album = albumPointer.data();
    if (!album) return;

    painter->save();

    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    // const bool isActive = index.data( ActiveItemRole ).toBool();
    const bool isHovered = index.data(Finder::HoveredItemRole ).toBool();
    const bool isSelected = option.state & QStyle::State_Selected;

    // thumb
    QPixmap pixmap = getAlbumPixmap(album);
    if (pixmap.isNull()) pixmap = getMissingAlbumPixmap();
    painter->drawPixmap(0, 0, pixmap);

    // play icon overlayed on the thumb
    if (isHovered) {
        double animation = index.data(Finder::PlayIconAnimationItemRole).toDouble();
        bool playIconHovered = index.data(Finder::PlayIconHoveredRole).toBool();
        paintPlayIcon(painter, line, animation, playIconHovered);
    }

    // name
    drawName(painter, option, album->getTitle(), line, isSelected);
    if (album->getYear() > 0) {
        drawBadge(painter, QString::number(album->getYear()), line);
    }

    painter->restore();

}

void FinderItemDelegate::paintFolder(QPainter* painter,
                                     const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const {

    // get the data object
    const QVariant dataObject = index.data(Finder::DataObjectRole);
    const FolderPointer folderPointer = dataObject.value<FolderPointer>();
    Folder *folder = folderPointer.data();
    if (!folder) return;

    // const bool isActive = index.data( ActiveItemRole ).toBool();
    const bool isHovered = index.data(Finder::HoveredItemRole ).toBool();
    const bool isSelected = option.state & QStyle::State_Selected;

    painter->save();
    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    // thumb
    painter->drawPixmap(0, 0, getMissingItemBackground(IconUtils::pixelRatio()));
    QIcon fileIcon = index.data(QFileSystemModel::FileIconRole).value<QIcon>();
    if (!fileIcon.isNull())
        painter->drawPixmap(ITEM_WIDTH/2-32, ITEM_HEIGHT/3-32, fileIcon.pixmap(QSize(64, 64)));

    // play icon overlayed on the thumb
    if (isHovered) {
        double animation = index.data(Finder::PlayIconAnimationItemRole).toDouble();
        bool playIconHovered = index.data(Finder::PlayIconHoveredRole).toBool();
        paintPlayIcon(painter, line, animation, playIconHovered);
    }

    QString trackLength;
    int totalLength = folder->getTotalLength();
    if (totalLength > 3600)
        trackLength =  QTime().addSecs(totalLength).toString("h:mm:ss");
    else if (totalLength > 0)
        trackLength = QTime().addSecs(totalLength).toString("m:ss");
    drawBadge(painter, trackLength, line);
    drawCentralLabel(painter, QString::number(folder->getTrackCount()), line);

    drawName(painter, option, folder->getName(), line, isSelected);

    painter->restore();

}

void FinderItemDelegate::paintTrack(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {

    // get the data object
    const QVariant dataObject = index.data(Finder::DataObjectRole);
    const TrackPointer trackPointer = dataObject.value<TrackPointer>();
    Track *track = trackPointer.data();
    if (!track) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // const bool isActive = index.data( ActiveItemRole ).toBool();
    const bool isHovered = index.data(Finder::HoveredItemRole ).toBool();
    const bool isSelected = option.state & QStyle::State_Selected;

    painter->save();
    painter->translate(option.rect.topLeft());
    const QRect line(0, 0, option.rect.width(), option.rect.height());

    // thumb
    painter->drawPixmap(0, 0, getMissingItemBackground(IconUtils::pixelRatio()));

    // play icon overlayed on the thumb
    if (isHovered) {
        double animation = index.data(Finder::PlayIconAnimationItemRole).toDouble();
        bool playIconHovered = index.data(Finder::PlayIconHoveredRole).toBool();
        paintPlayIcon(painter, line, animation, playIconHovered);
    }

    QString trackLength;
    if (track->getLength() > 3600)
        trackLength =  QTime().addSecs(track->getLength()).toString("h:mm:ss");
    else if (track->getLength() > 0)
        trackLength = QTime().addSecs(track->getLength()).toString("m:ss");
    drawBadge(painter, trackLength, line);
    int trackNumber = track->getNumber();
    if (trackNumber > 0)
        drawCentralLabel(painter, QString::number(trackNumber), line);
    drawName(painter, option, track->getTitle(), line, isSelected);

    painter->restore();

}

void FinderItemDelegate::paintPlayIcon(QPainter *painter, const QRect& rect, double opacity, bool isHovered) const {
    const QPixmap &playIcon = getPlayIcon(isHovered);

    painter->save();
    const qreal pixelRatio = IconUtils::pixelRatio();
    painter->translate((rect.width() - playIcon.width() - PADDING) * pixelRatio, PADDING * pixelRatio);

    if (isHovered)
        painter->setOpacity(opacity * .75);
    else
        painter->setOpacity(.75);

    painter->drawPixmap(0, 0, playIcon);

    if (isHovered) {
        painter->setOpacity(.9 - opacity * .9);
        painter->drawPixmap(0, 0, playIcon);
    }

    painter->restore();
}

void FinderItemDelegate::drawName(QPainter *painter, const QStyleOptionViewItem &option, QString name, const QRect& rect, bool isSelected) const {

    QRect nameBox = rect;
    nameBox.adjust(0, 0, 0, -ITEM_HEIGHT*2/3);
    nameBox.translate(0, ITEM_HEIGHT - nameBox.height());

    painter->save();
    painter->setPen(Qt::NoPen);
    if (isSelected) {
        painter->setOpacity(.9);
        painter->setBrush(option.palette.highlight());
    } else {
        painter->setBrush(QColor(0, 0, 0, 128));
    }

    painter->drawRect(nameBox);
    painter->restore();

    painter->save();

    bool tooBig = false;
    QRect textBox = painter->boundingRect(nameBox, Qt::AlignCenter | Qt::TextWordWrap, name);
    if (textBox.height() >= nameBox.height()) {
        painter->setFont(FontUtils::small());
        textBox = painter->boundingRect(nameBox, Qt::AlignCenter | Qt::TextWordWrap, name);
        if (textBox.height() > nameBox.height()) {
            // it's still too big
            painter->setClipRect(nameBox);
            tooBig = true;
        }
    }

    if (isSelected)
        painter->setPen(QPen(option.palette.highlightedText(), 0));

    if (tooBig)
        painter->drawText(nameBox, Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, name);
    else
        painter->drawText(textBox, Qt::AlignCenter | Qt::TextWordWrap, name);

    painter->restore();
}

void FinderItemDelegate::drawBadge(QPainter *painter, const QString &text,  const QRect& rect) const {
    static const int PADDING = 4;

    painter->save();
    painter->setFont(FontUtils::small());
    QRectF textBox = painter->boundingRect(rect, Qt::AlignLeft | Qt::AlignTop, text);
    textBox.adjust(0, 0, PADDING, 0);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 128));
    painter->drawRect(textBox);

    painter->setPen(Qt::white);
    painter->drawText(textBox, Qt::AlignCenter, text);
    painter->restore();
}

void FinderItemDelegate::drawCentralLabel(QPainter *painter, const QString &text,  const QRect& rect) const {
    static const int PADDING = 10;

    painter->save();
    painter->setFont(FontUtils::small());
    QSizeF textSize(QFontMetrics(painter->font()).size( Qt::TextSingleLine, text));
    QRect textBox((rect.width() - textSize.width()) / 2 , (rect.height() - textSize.height()) / 3 + 4, textSize.width(), textSize.height());

    QRect roundedRect = textBox;
    roundedRect.adjust(-PADDING/2, -PADDING/3, PADDING/2, PADDING/3);

    painter->setRenderHints(QPainter::Antialiasing, true);
    painter->setBrush(QColor(0, 0, 0, 96));
    painter->setPen(QColor(255, 255, 255, 224));
    painter->drawRoundedRect(roundedRect, PADDING/2, PADDING/2, Qt::AbsoluteSize);
    painter->drawText(textBox, Qt::AlignCenter, text);
    painter->restore();

}

QPixmap FinderItemDelegate::getArtistPixmap(Artist *artist) const {
    const qreal pixelRatio = IconUtils::pixelRatio();
    QByteArray cacheKeyArray = QString("p" + QString::number(pixelRatio)).toLocal8Bit();
    const char* cacheKey = cacheKeyArray.constData();

    QPixmap pixmap = artist->property(cacheKey).value<QPixmap>();
    if (pixmap.isNull()) {
        // qDebug() << "Creating pixmap for" << artist;
        QPixmap p = artist->getPhoto();
        int xOffset = 0;
        int wDiff = p.width() - ITEM_WIDTH * pixelRatio;
        if (wDiff > 0) xOffset = wDiff / 2;
        int yOffset = 0;
        int hDiff = p.height() - ITEM_HEIGHT * pixelRatio;
        if (hDiff > 0) yOffset = hDiff / 4;
        pixmap = p.copy(xOffset, yOffset, ITEM_WIDTH * pixelRatio, ITEM_HEIGHT * pixelRatio);
        pixmap.setDevicePixelRatio(pixelRatio);
        artist->setProperty(cacheKey, pixmap);
    }
    return pixmap;
}

QPixmap FinderItemDelegate::getAlbumPixmap(Album *album) const {
    const qreal pixelRatio = IconUtils::pixelRatio();
    QByteArray cacheKeyArray = QString("p" + QString::number(pixelRatio)).toLocal8Bit();
    const char* cacheKey = cacheKeyArray.constData();

    QPixmap pixmap = album->property(cacheKey).value<QPixmap>();
    if (pixmap.isNull()) {
        // qDebug() << "Creating pixmap for" << album->getTitle();
        QPixmap p = album->getPhoto();
        if (p.isNull()) {
            // qWarning() << "Missing photo for" << album->getTitle();
            return pixmap;
        }
        pixmap = p.scaled(ITEM_WIDTH * pixelRatio, ITEM_HEIGHT * pixelRatio);
        pixmap.setDevicePixelRatio(pixelRatio);
        album->setProperty(cacheKey, pixmap);
    }
    return pixmap;
}

QPixmap FinderItemDelegate::getFolderPixmap(Folder *folder) const {
    QPixmap pixmap = folder->property("pixmap").value<QPixmap>();
    if (pixmap.isNull()) {
        // qDebug() << "Creating pixmap for" << folder;
        QImage p = folder->getPhoto();
        int xOffset = 0;
        int wDiff = p.width() - ITEM_WIDTH;
        if (wDiff > 0) xOffset = wDiff / 2;
        int yOffset = 0;
        int hDiff = p.height() - ITEM_HEIGHT;
        if (hDiff > 0) yOffset = hDiff / 4;
        pixmap = QPixmap::fromImage(p.copy(xOffset, yOffset, ITEM_WIDTH, ITEM_HEIGHT));
        folder->setProperty("pixmap", pixmap);
    }
    return pixmap;
}
