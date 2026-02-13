#include "PlaylistWindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPixmap>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QFile>
#include <QPainter>

PlaylistWindow::PlaylistWindow(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);

    connect(ui.btnBack, &QPushButton::clicked,
            this, &PlaylistWindow::requestBack);
    connect(ui.btnUp, &QPushButton::clicked,
            this, &PlaylistWindow::scrollUp);
    connect(ui.btnDown, &QPushButton::clicked,
            this, &PlaylistWindow::scrollDown);

    createItems();
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

        // Обложка трека с вертикальным выравниванием по центру
        // Размер обложки: высота бокса (52) - 5 пикселей = 47x47 (квадратная)
        it.cover = new QLabel;
        it.cover->setFixedSize(47, 47);
        it.cover->setScaledContents(false);  // Отключаем автоматическое масштабирование
        it.cover->setAlignment(Qt::AlignCenter);
        
        // Обёртка для обложки с вертикальным выравниванием
        it.coverWrapper = new QWidget;
        auto* coverLayout = new QVBoxLayout(it.coverWrapper);
        coverLayout->setContentsMargins(0, 0, 0, 0);
        coverLayout->setAlignment(Qt::AlignVCenter);
        coverLayout->addWidget(it.cover);

        auto* v = new QVBoxLayout;
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(2);
        
        it.title = new QLabel;
        it.title->setWordWrap(false);
        it.meta = new QLabel;
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
                                 const QString& name)
{
    allTracks = tracks;
    startIndex = 0;
    ui.lblPlaylistTitle->setText(name);
    updateView();
}

QPixmap PlaylistWindow::loadCover(const QString& coverPath)
{
    if (coverPath.isEmpty()) {
        // Пустая обложка - возвращаем пустой QPixmap
        return QPixmap();
    }

    // Если это HTTP URL, пока не поддерживаем загрузку
    if (coverPath.startsWith("http://") || coverPath.startsWith("https://")) {
        return QPixmap();
    }

    // Относительный путь к файлу обложки от корня проекта
    QDir dir(QCoreApplication::applicationDirPath());
    dir.cdUp(); // из build dir (например, cmake-build-debug)
    dir.cdUp(); // к repo root (rspl)

    QString fullPath;
    
    // Если путь абсолютный (Windows: C:\...), пытаемся извлечь относительную часть
    if (coverPath.length() > 2 && coverPath.at(1) == QChar(':')) {
        // Абсолютный путь Windows - ищем часть после корня проекта
        QString repoRoot = dir.absolutePath();
        QString normalizedPath = QDir::fromNativeSeparators(coverPath);
        QString normalizedRoot = QDir::fromNativeSeparators(repoRoot);
        
        if (normalizedPath.startsWith(normalizedRoot, Qt::CaseInsensitive)) {
            // Извлекаем относительный путь
            QString relative = normalizedPath.mid(normalizedRoot.length());
            relative.replace('\\', '/');
            if (relative.startsWith('/')) {
                relative = relative.mid(1);
            }
            fullPath = dir.absoluteFilePath(relative);
        } else {
            // Если не удалось найти корень проекта в пути, пробуем как есть
            fullPath = normalizedPath;
        }
    } else {
        // Относительный путь - добавляем к корню проекта
        QString normalizedPath = coverPath;
        normalizedPath.replace('\\', '/');
        fullPath = dir.absoluteFilePath(normalizedPath);
    }

    QFileInfo fileInfo(fullPath);
    if (fileInfo.exists() && fileInfo.isFile()) {
        QPixmap pixmap(fullPath);
        if (!pixmap.isNull()) {
            // Масштабируем с сохранением пропорций до размера, где меньшая сторона = 47
            QPixmap scaled = pixmap.scaled(47, 47, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // Если изображение не квадратное, создаем квадратное изображение 47x47
            if (scaled.width() != 47 || scaled.height() != 47) {
                QPixmap square(47, 47);
                square.fill(Qt::transparent);
                QPainter painter(&square);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                int x = (47 - scaled.width()) / 2;
                int y = (47 - scaled.height()) / 2;
                painter.drawPixmap(x, y, scaled);
                painter.end();
                return square;
            }
            return scaled;
        }
    }

    // Если не удалось загрузить, возвращаем пустой QPixmap
    return QPixmap();
}

void PlaylistWindow::updateView()
{
    for (int i = 0; i < items.size(); ++i) {
        int idx = startIndex + i;
        if (idx < allTracks.size()) {
            items[i].box->setVisible(true);
            
            // Устанавливаем текст через бегущие строки
            items[i].titleMarquee->setText(allTracks[idx].title);
            items[i].metaMarquee->setText(allTracks[idx].meta);
            
            // Загружаем обложку только если путь не пустой
            if (!allTracks[idx].coverPath.isEmpty()) {
                QPixmap cover = loadCover(allTracks[idx].coverPath);
                if (!cover.isNull()) {
                    // loadCover уже возвращает изображение размером 47x47
                    items[i].cover->setPixmap(cover);
                    items[i].cover->setVisible(true);
                } else {
                    items[i].cover->setPixmap(QPixmap());
                    items[i].cover->setVisible(false);
                }
            } else {
                // Если путь пустой (например, для плейлистов), скрываем обложку
                items[i].cover->setPixmap(QPixmap());
                items[i].cover->setVisible(false);
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
