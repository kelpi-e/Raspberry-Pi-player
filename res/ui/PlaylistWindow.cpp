#include "PlaylistWindow.h"
#include <QPushButton>

PlaylistWindow::PlaylistWindow(QWidget* parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    setFixedSize(240, 320);
    
    // Подключение кнопки "назад"
    connect(ui.btnBack, &QPushButton::clicked, this, &PlaylistWindow::requestBack);
    
    // Настройки таблицы
    ui.tblTracks->horizontalHeader()->setVisible(true);
    ui.tblTracks->verticalHeader()->setVisible(false);
    ui.tblTracks->setAlternatingRowColors(true);
    ui.tblTracks->setStyleSheet(
        "QTableWidget::item { padding: 4px; }"
        "QTableWidget::item:selected { background-color: #1db954; }"
    );
}

void PlaylistWindow::setPlaylist(const QList<QPair<QString, QString>>& tracks)
{
    ui.tblTracks->setRowCount(tracks.size());
    for (int i = 0; i < tracks.size(); ++i) {
        ui.tblTracks->setItem(i, 0, new QTableWidgetItem(tracks[i].first));
        ui.tblTracks->setItem(i, 1, new QTableWidgetItem(tracks[i].second));
        ui.tblTracks->setItem(i, 2, new QTableWidgetItem("0:00"));
    }
}
