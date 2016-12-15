cmake_minimum_required(VERSION 3.6)
project(Demo_aircondition)

set(CMAKE_CXX_STANDARD 11)

# REQUIRED 自动找需要的
find_package(VTK REQUIRED
        )
include(${VTK_USE_FILE})

set(SOURCE_FILES
        main.cpp
        WindActorWrapper.cpp
        )
add_executable(Demo_aircondition ${SOURCE_FILES})
target_link_libraries(Demo_aircondition ${VTK_LIBRARIES})
