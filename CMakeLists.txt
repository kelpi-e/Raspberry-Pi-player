cmake_minimum_required(VERSION 3.16)
project(untitled3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

# Подключаем заголовки TagLib вручную
include_directories(/usr/include/taglib)

set(UI_FILES res/ui/PlayerWindow.ui)

add_executable(untitled3
        res/main.cpp
        res/ui/PlayerWindow.cpp
        res/ui/PlayerWindow.h
        ${UI_FILES}
        res/audio/PlayerAudio.cpp
        res/audio/PlayerAudio.h
)

target_include_directories(untitled3 PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Линкуем libtag.so — она называется просто "tag"
target_link_libraries(untitled3
        Qt6::Core
        Qt6::Widgets
        Qt6::Multimedia
        tag
)

# Sanitizer (по желанию)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(untitled3 PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(untitled3 PRIVATE -fsanitize=address)
endif()
