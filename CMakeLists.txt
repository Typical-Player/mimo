cmake_minimum_required(VERSION 3.20)
project(mimoapp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Multimedia)

qt_standard_project_setup(REQUIRES 6.10)

qt_add_executable(mimoapp
        src/main.cpp
        src/characteristicvector.h
        src/characteristicvector.cpp
        src/sceneworker.h
        src/sceneworker.cpp
        src/videocontroller.h
        src/videocontroller.cpp
        src/scenechangemodel.h
        src/scenechangemodel.cpp
        src/minichartitem.cpp
        src/minichartitem.h
)

qt_add_qml_module(mimoapp
        URI mimo
        VERSION 1.0
        QML_FILES
        qml/Main.qml
        qml/VideoOverlay.qml
        qml/SceneChangePanel.qml
        RESOURCES
        assets/Logo.png
)

set_target_properties(mimoapp PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER com.typical.mimoapp
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(mimoapp PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Multimedia
)

target_include_directories(mimoapp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (EMSCRIPTEN)
    set_target_properties(mimoapp PROPERTIES
            QT_WASM_INITIAL_MEMORY 512MB
    )
endif ()

include(GNUInstallDirs)
install(TARGETS mimoapp
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
