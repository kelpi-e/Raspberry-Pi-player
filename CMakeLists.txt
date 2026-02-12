cmake_minimum_required(VERSION 3.16)
project(Raspberry-Pi-player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Multimedia
)
find_package(TagLib REQUIRED)

add_executable(Raspberry-Pi-player
        res/main.cpp
        res/resources.qrc  # Добавить здесь

        res/ui/PlayerWindow.cpp
        res/ui/PlayerScene.cpp
        res/ui/PlaylistScene.cpp
        res/ui/PlaylistWindow.cpp

        res/audio/PlayerAudio.cpp

        res/utils/wifichecker.cpp
        res/utils/xraycontroller.cpp
        res/utils/MarqueeController.cpp
        res/utils/godSays.cpp
        res/utils/FullscreenMessage.cpp

        res/ui/PlayerWindow.ui
        res/ui/PlaylistWindow.ui
)

# Убрать qt_add_resources - теперь это делает AUTORCC

target_include_directories(Raspberry-Pi-player
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Raspberry-Pi-player
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        TagLib::TagLib
)