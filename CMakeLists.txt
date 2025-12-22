cmake_minimum_required(VERSION 3.16)
project(Raspberry-Pi-player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
        Core
        Widgets
        Multimedia
        WebEngineWidgets
)

qt_add_resources(player_resources res/ui/icons/resources.qrc)

add_executable(Raspberry-Pi-player
        res/main.cpp
        res/ui/PlayerWindow.cpp
        res/ui/PlayerWindow.h
        res/ui/PlayerScene.cpp
        res/ui/PlayerScene.h
        res/ui/SpotifyScene.cpp
        res/ui/SpotifyScene.h
        res/audio/PlayerAudio.cpp
        res/audio/PlayerAudio.h
        res/utils/wifichecker.cpp
        res/utils/wifichecker.h
        res/utils/xraycontroller.cpp
        res/utils/xraycontroller.h
        res/utils/MarqueeController.cpp
        res/utils/MarqueeController.h
        ${player_resources}
)

target_include_directories(Raspberry-Pi-player
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(Raspberry-Pi-player
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::WebEngineWidgets
        tag
)
