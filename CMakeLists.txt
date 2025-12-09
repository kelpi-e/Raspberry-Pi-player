cmake_minimum_required(VERSION 3.16)
project(Raspberry-Pi-player)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Использование vcpkg для управления зависимостями
# При запуске CMake используйте: cmake -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)
find_package(taglib REQUIRED)

set(UI_FILES res/ui/PlayerWindow.ui)

add_executable(Raspberry-Pi-player
        res/main.cpp
        res/ui/PlayerWindow.cpp
        res/ui/PlayerWindow.h
        ${UI_FILES}
        res/audio/PlayerAudio.cpp
        res/audio/PlayerAudio.h
)

target_include_directories(Raspberry-Pi-player PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(Raspberry-Pi-player
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        taglib::taglib
)
