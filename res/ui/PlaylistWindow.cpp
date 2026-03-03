#include "PlaylistWindow.h"
#include "../utils/ThemeLoader.h"
#include "../pch.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include <QSizePolicy>
#include <QColor>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QPainter>

PlaylistWindow::PlaylistWindow(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ThemeLoader::applyPlaylistWindow(this);

    connect(ui.btnBack, &QPushButton::clicked,
            this, &PlaylistWindow::requestBack);
    connect(ui.btnUp, &QPushButton::clicked,
            this, &PlaylistWindow::scrollUp);
    connect(ui.btnDown, &QPushButton::clicked,
            this, &PlaylistWindow::scrollDown);

    createItems();

    // Ограничение по ширине 240: контент 236px (отступы 2+2)
    const int contentW = WIDTH - 4;
    ui.lblPlaylistTitle->setMaximumWidth(contentW - 36 - 6);
    ui.lblPlaylistTitle->setSizePolicy(QSizePolicy::Expanding, ui.lblPlaylistTitle->sizePolicy().verticalPolicy());
    for (Item& it : items) {
        it.box->setMaximumWidth(contentW);
        const int textColW = contentW - 4 - 44 - 8 - 4;
        it.title->setMaximumWidth(textColW);
        it.meta->setMaximumWidth(textColW);
    }
    const int navBtnW = (contentW - 4) / 2;
    ui.btnUp->setFixedSize(navBtnW, 28);
    ui.btnDown->setFixedSize(navBtnW, 28);
}

PlaylistWindow::~PlaylistWindow()
{
    // MarqueeController удалится автоматически, т.к. родитель - QLabel
}

void PlaylistWindow::createItems()
{
    for (int i = 0; i < 3; ++i) {
        Item it;
        // Use a flat button as a clickable row container
        auto* btn = new QPushButton;
        btn->setFlat(true);
        btn->setCheckable(false);
        btn->setAutoFillBackground(false);
        btn->setCursor(Qt::PointingHandCursor);
        // Сохраняем высоту строки примерно как раньше
        btn->setMinimumHeight(52);
        btn->setMaximumHeight(52);
        it.box = btn;

        auto* h = new QHBoxLayout(btn);
        h->setContentsMargins(4, 0, 4, 0);
        h->setSpacing(8);

        // Обложка трека: строго 44x44 внутри панели 52px (без переполнения)
        const int coverSz = 44;
        it.cover = new QLabel;
        it.cover->setObjectName(QStringLiteral("cover"));
        it.cover->setFixedSize(coverSz, coverSz);
        it.cover->setScaledContents(false);
        it.cover->setAlignment(Qt::AlignCenter);
        it.cover->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        it.coverWrapper = new QWidget;
        it.coverWrapper->setFixedSize(coverSz, 52);
        it.coverWrapper->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        auto* coverLayout = new QVBoxLayout(it.coverWrapper);
        coverLayout->setContentsMargins(0, 0, 0, 0);
        coverLayout->setAlignment(Qt::AlignVCenter);
        coverLayout->addWidget(it.cover);

        auto* v = new QVBoxLayout;
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(2);
        
        it.title = new QLabel;
        it.title->setObjectName(QStringLiteral("trackTitle"));
        it.title->setWordWrap(false);
        it.meta = new QLabel;
        it.meta->setObjectName(QStringLiteral("trackMeta"));
        it.meta->setWordWrap(false);

        // Создаём бегущие строки для title и meta
        it.titleMarquee = new MarqueeController(it.title, 150);
        it.metaMarquee = new MarqueeController(it.meta, 150);

        v->addWidget(it.title);
        v->addWidget(it.meta);

        h->addWidget(it.coverWrapper);
        h->addLayout(v);

        ui.vLayTracks->addWidget(it.box);

        // Click on the whole row -> emit itemClicked with logical index
        connect(btn, &QPushButton::clicked, this, [this, rowIndex = i]() {
            const int logicalIndex = startIndex + rowIndex;
            emit itemClicked(logicalIndex);
        });

        items.append(it);
    }
}

void PlaylistWindow::setPlaylist(const QList<TrackInfo>& tracks,
                                 const QString& name,
                                 bool showCoversMode)
{
    allTracks = tracks;
    startIndex = 0;
    showCovers = showCoversMode;
    ui.lblPlaylistTitle->setText(name);
    updateView();
}

QPixmap PlaylistWindow::loadCover(const QString& coverPath)
{
    constexpr int coverSize = 44;
    const QColor letterboxColor(0x1e, 0x1e, 0x1e);

    const auto scaleToCover = [letterboxColor](QPixmap pixmap) -> QPixmap {
        if (pixmap.isNull()) return QPixmap();
        QPixmap scaled = pixmap.scaled(coverSize, coverSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        if (scaled.width() == coverSize && scaled.height() == coverSize) return scaled;
        QPixmap square(coverSize, coverSize);
        square.fill(letterboxColor);
        QPainter painter(&square);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap((coverSize - scaled.width()) / 2, (coverSize - scaled.height()) / 2, scaled);
        painter.end();
        return square;
    };

    const auto defaultPixmap = [&scaleToCover]() -> QPixmap {
        QIcon icon(QStringLiteral(":/res/ui/icons/default.svg"));
        if (!icon.isNull()) {
            QPixmap pm = icon.pixmap(coverSize, coverSize);
            return pm.isNull() ? QPixmap() : scaleToCover(pm);
        }
        QPixmap pm(QStringLiteral(":/res/ui/icons/default.svg"));
        return pm.isNull() ? QPixmap() : scaleToCover(pm);
    };

    if (coverPath.isEmpty()) {
        return defaultPixmap();
    }

    if (coverPath.startsWith("http://") || coverPath.startsWith("https://")) {
        return defaultPixmap();
    }

    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp();
    dir.cdUp();
    QString repoRoot = dir.absolutePath();
    QString normalizedPath = QDir::fromNativeSeparators(coverPath).replace('\\', '/');

    QStringList pathsToTry;
    if (normalizedPath.length() > 2 && normalizedPath.at(1) == QChar(':')) {
        pathsToTry << normalizedPath;
        QString rootNorm = QDir::fromNativeSeparators(repoRoot);
        if (normalizedPath.startsWith(rootNorm, Qt::CaseInsensitive)) {
            QString rel = normalizedPath.mid(rootNorm.length()).trimmed();
            if (rel.startsWith('/')) rel = rel.mid(1);
            pathsToTry << dir.absoluteFilePath(rel);
        }
    } else {
        pathsToTry << dir.absoluteFilePath(normalizedPath);
        if (!normalizedPath.startsWith("web-interface/")) {
            pathsToTry << dir.absoluteFilePath("web-interface/" + normalizedPath);
        }
    }

    for (const QString& fullPath : pathsToTry) {
        QFileInfo fi(fullPath);
        if (!fi.exists() || !fi.isFile()) continue;
        QPixmap pixmap(fullPath);
        if (!pixmap.isNull())
            return scaleToCover(pixmap);
    }

    return defaultPixmap();
}

void PlaylistWindow::updateView()
{
    for (int i = 0; i < items.size(); ++i) {
        int idx = startIndex + i;
        if (idx < allTracks.size()) {
            items[i].box->setVisible(true);
            items[i].coverWrapper->setVisible(showCovers);

            // Устанавливаем текст через бегущие строки (название и мета — всегда)
            items[i].titleMarquee->setText(allTracks[idx].title);
            items[i].metaMarquee->setText(allTracks[idx].meta);

            if (showCovers) {
                // Для треков: обложка из downloaded_covers или default.svg при отсутствии
                items[i].cover->setPixmap(loadCover(allTracks[idx].coverPath));
                items[i].cover->setVisible(true);
            }
        } else {
            items[i].box->setVisible(false);
        }
    }
}

void PlaylistWindow::scrollUp()
{
    if (startIndex > 0) {
        startIndex--;
        updateView();
    }
}

void PlaylistWindow::scrollDown()
{
    if (startIndex + 3 < allTracks.size()) {
        startIndex++;
        updateView();
    }
}
