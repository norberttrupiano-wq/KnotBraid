cmake_minimum_required(VERSION 3.16)

project(KnotBraidLauncher VERSION 0.2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(knotbraid_first_existing_path out_var)
    set(found_path "")
    foreach(candidate IN ITEMS ${ARGN})
        if(EXISTS "${candidate}")
            set(found_path "${candidate}")
            break()
        endif()
    endforeach()
    set(${out_var} "${found_path}" PARENT_SCOPE)
endfunction()

file(GLOB KNOTBRAID_LOGIKNOTTING_TRANSLATION_QM_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../LogiKnotting/translations/*.qm"
)

knotbraid_first_existing_path(KNOTBRAID_DOC_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs"
)
if(KNOTBRAID_DOC_DIR)
    file(GLOB KNOTBRAID_DOC_PDF_FILES CONFIGURE_DEPENDS
        "${KNOTBRAID_DOC_DIR}/*.pdf"
    )
endif()

knotbraid_first_existing_path(KNOTBRAID_TUTORIAL_ASSET_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/tutorial"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/Tutorial"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/Tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/Videos Tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../docs/Vidéos Tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/tutorial"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/Tutorial"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/Tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/Videos Tutoriel"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Docs/Vidéos Tutoriel"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport CoreTools Multimedia MultimediaWidgets)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../LogiBraiding"
                 "${CMAKE_CURRENT_BINARY_DIR}/LogiBraiding"
                 EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../LogiKnotting"
                 "${CMAKE_CURRENT_BINARY_DIR}/LogiKnotting"
                 EXCLUDE_FROM_ALL)

set(KNOTBRAID_SHARED_RESOURCES
    resources/resources.qrc
)

set(KNOTBRAID_SHELL_SOURCES
    src/shell_main.cpp
    src/ShellMainWindow.h
    src/ShellMainWindow.cpp
    ${KNOTBRAID_SHARED_RESOURCES}
    ../LogiBraiding/resources/logibraiding_resources.qrc
    ../LogiKnotting/resources/logiknotting_resources.qrc
)

set(KNOTBRAID_LAUNCHER_SOURCES
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    ${KNOTBRAID_SHARED_RESOURCES}
)

if(WIN32)
    set(KNOTBRAID_WINDOWS_RESOURCES
        resources/windows/knotbraid.rc
    )
endif()

if(WIN32)
    function(knotbraid_add_qt_deploy target_name)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND "$<TARGET_FILE:Qt6::windeployqt>"
                --release
                --force
                --compiler-runtime
                "$<TARGET_FILE:${target_name}>"
            WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target_name}>"
            COMMENT "Deploying Qt runtime for ${target_name}"
            VERBATIM
        )
    endfunction()
endif()

qt_add_executable(KnotBraid
    WIN32
    MANUAL_FINALIZATION
    ${KNOTBRAID_SHELL_SOURCES}
    ${KNOTBRAID_WINDOWS_RESOURCES}
)

qt_add_executable(KnotBraidLauncher
    WIN32
    MANUAL_FINALIZATION
    ${KNOTBRAID_LAUNCHER_SOURCES}
    ${KNOTBRAID_WINDOWS_RESOURCES}
)

target_link_libraries(KnotBraid PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::PrintSupport
    LogiBraidingAppLib
    LogiKnottingAppLib
)
target_compile_definitions(KnotBraid PRIVATE QT_NO_CAST_FROM_ASCII)

target_link_libraries(KnotBraidLauncher PRIVATE Qt6::Widgets)
target_compile_definitions(KnotBraidLauncher PRIVATE QT_NO_CAST_FROM_ASCII)

if(WIN32)
    target_link_libraries(KnotBraidLauncher PRIVATE version)
endif()

if(WIN32)
    knotbraid_add_qt_deploy(KnotBraid)
    knotbraid_add_qt_deploy(KnotBraidLauncher)
endif()

if(KNOTBRAID_LOGIKNOTTING_TRANSLATION_QM_FILES)
    add_custom_command(TARGET KnotBraid POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:KnotBraid>/translations"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${KNOTBRAID_LOGIKNOTTING_TRANSLATION_QM_FILES}
            "$<TARGET_FILE_DIR:KnotBraid>/translations"
        COMMENT "Copying LogiKnotting translations for KnotBraid"
        VERBATIM
    )
endif()

if(KNOTBRAID_DOC_PDF_FILES)
    add_custom_command(TARGET KnotBraid POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:KnotBraid>/docs"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${KNOTBRAID_DOC_PDF_FILES}
            "$<TARGET_FILE_DIR:KnotBraid>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${KNOTBRAID_DOC_PDF_FILES}
            "$<TARGET_FILE_DIR:KnotBraid>/docs"
        COMMENT "Copying KnotBraid PDF documentation for KnotBraid"
        VERBATIM
    )
endif()

if(KNOTBRAID_TUTORIAL_ASSET_DIR)
    add_custom_command(TARGET KnotBraid POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:KnotBraid>/docs"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:KnotBraid>/docs/tutorial"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:KnotBraid>/docs/Tutorial"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:KnotBraid>/docs/Tutoriel"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:KnotBraid>/docs/Vidéos Tutoriel"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:KnotBraid>/docs/Videos Tutoriel"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${KNOTBRAID_TUTORIAL_ASSET_DIR}"
            "$<TARGET_FILE_DIR:KnotBraid>/docs/tutorial"
        COMMENT "Copying KnotBraid tutorial assets for KnotBraid"
        VERBATIM
    )
endif()

qt_finalize_executable(KnotBraid)
qt_finalize_executable(KnotBraidLauncher)
