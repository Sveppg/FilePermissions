cmake_minimum_required(VERSION 3.16)

project(PermissionGui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(PermissionGui
    main.cpp
)

target_link_libraries(PermissionGui
    Qt6::Widgets
)