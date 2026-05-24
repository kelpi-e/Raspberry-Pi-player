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
        Svg
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

add_executable(Raspberry-Pi-player
        res/main.cpp
        res/resources.qrc

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
        res/utils/ThemeLoader.cpp

        res/ui/PlayerWindow.ui
        res/ui/PlaylistWindow.ui
)

target_include_directories(Raspberry-Pi-player
        PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${TAGLIB_INCLUDE_DIRS}
)

target_link_libraries(Raspberry-Pi-player
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::Svg
        ${TAGLIB_LIBRARIES}
)