cmake_minimum_required(VERSION 3.16)
project(SAV_535 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GUI "Build Qt GUI" OFF)

add_library(sav_core
    src/Instruction.cpp
    src/Parser.cpp
    src/Assembler.cpp
    src/Memory.cpp
    src/L2Cache.cpp
    src/Cache.cpp
    src/Simulator.cpp
    src/UI.cpp
)

target_include_directories(sav_core PUBLIC src)
target_compile_features(sav_core PUBLIC cxx_std_17)

add_executable(sim_cli src/main.cpp)
target_link_libraries(sim_cli PRIVATE sav_core)
target_include_directories(sim_cli PRIVATE src)

if(BUILD_GUI)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    add_executable(sim_gui
        src/gui/main_gui.cpp
        src/gui/MainWindow.cpp
    )
    target_link_libraries(sim_gui PRIVATE sav_core Qt6::Widgets)
    target_include_directories(sim_gui PRIVATE src src/gui)
endif()
