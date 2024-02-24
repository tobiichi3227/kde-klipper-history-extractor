cmake_minimum_required(VERSION 3.25)

project(KlipperHistoryFileExtractor LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


find_package(Qt6 COMPONENTS Core Gui REQUIRED)
find_package(Qt5 COMPONENTS Core Gui REQUIRED)

# qt_standard_project_setup()

add_executable(extractor src/main.cpp)

if (Qt6_FOUND)
    target_link_libraries(extractor PRIVATE Qt6::Core Qt6::Gui)
elseif (Qt5_FOUND)
    target_link_libraries(extractor PRIVATE Qt5::Core Qt5::Gui)
endif ()