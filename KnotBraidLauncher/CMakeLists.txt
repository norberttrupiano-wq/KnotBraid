cmake_minimum_required(VERSION 3.16)

project(KnotBraidLauncher VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(KNOTBRAID_LAUNCHER_SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(KnotBraid
        WIN32
        MANUAL_FINALIZATION
        ${KNOTBRAID_LAUNCHER_SOURCES}
    )
else()
    add_executable(KnotBraid
        WIN32
        ${KNOTBRAID_LAUNCHER_SOURCES}
    )
endif()

target_link_libraries(KnotBraid PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(KnotBraid PRIVATE QT_NO_CAST_FROM_ASCII)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(KnotBraid)
endif()
