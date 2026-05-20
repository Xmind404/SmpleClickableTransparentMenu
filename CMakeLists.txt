cmake_minimum_required(VERSION 3.20)
project(CebulaWare CXX)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE ALL_SOURCES
    "src/*.cpp"
    "include/*.cpp"
)

add_executable(${PROJECT_NAME} WIN32 ${ALL_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    include
    include/ImGui
    include/ImGui/backends
)

target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 d3dcompiler dwmapi)
