cmake_minimum_required(VERSION 3.16)
project(Raspberry-Pi-player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Найти Qt6 компоненты
find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Multimedia
        WebEngineWidgets
)

# Ресурсы
qt_add_resources(player_resources res/ui/icons/resources.qrc)

# UI файлы
set(UI_FILES
        res/ui/PlayerWindow.ui
        res/ui/PlaylistWindow.ui
)

# Исходники
set(SRC_FILES
        res/main.cpp

        res/ui/PlayerWindow.cpp
        res/ui/PlayerScene.cpp
        res/ui/SpotifyScene.cpp
        res/ui/PlaylistScene.cpp
        res/ui/PlaylistWindow.cpp

        res/audio/PlayerAudio.cpp

        res/utils/wifichecker.cpp
        res/utils/xraycontroller.cpp
        res/utils/MarqueeController.cpp
)

# Заголовки
set(HEADERS
        res/ui/PlayerWindow.h
        res/ui/PlayerScene.h
        res/ui/SpotifyScene.h
        res/ui/PlaylistScene.h
        res/ui/PlaylistWindow.h

        res/audio/PlayerAudio.h

        res/utils/wifichecker.h
        res/utils/xraycontroller.h
        res/utils/MarqueeController.h
        res/utils/godSays.cpp
        res/utils/godSays.h
        res/utils/FullscreenMessage.cpp
        res/utils/FullscreenMessage.h
)

# Собрать исполняемый файл
add_executable(Raspberry-Pi-player
        ${SRC_FILES}
        ${HEADERS}
        ${UI_FILES}
        ${player_resources}
)

# Включаем директорию с сгенерированными файлами (ui_*.h)
target_include_directories(Raspberry-Pi-player
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

# Линковка Qt6 библиотек
target_link_libraries(Raspberry-Pi-player
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::WebEngineWidgets
        tag
)
