cmake_minimum_required(VERSION 3.16)

project(ChessGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Qml
    Quick
    Concurrent
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- Collect ONLY real source directories ----

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/AI/*.cpp"
    "${PROJECT_SOURCE_DIR}/board/*.cpp"
    "${PROJECT_SOURCE_DIR}/game/*.cpp"
    "${PROJECT_SOURCE_DIR}/moves/*.cpp"
    "${PROJECT_SOURCE_DIR}/pieces/*.cpp"
    "${PROJECT_SOURCE_DIR}/main.cpp"
    "${PROJECT_SOURCE_DIR}/domain/*.cpp"
)

file(GLOB_RECURSE HDR_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/AI/*.h"
    "${PROJECT_SOURCE_DIR}/board/*.h"
    "${PROJECT_SOURCE_DIR}/game/*.h"
    "${PROJECT_SOURCE_DIR}/moves/*.h"
    "${PROJECT_SOURCE_DIR}/pieces/*.h"
    "${PROJECT_SOURCE_DIR}/domain/*.h"
)

set(QML_RESOURCES
    qml.qrc
)

add_executable(${PROJECT_NAME}
    ${SRC_FILES}
    ${HDR_FILES}
    ${QML_RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::Concurrent
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}
)

if (WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()
