cmake_minimum_required(VERSION 3.31.3)
project(fisch-macro VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(lib/opencv/)
add_subdirectory(lib/imgui/)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource.res COMMAND windres src/resource.rc ${CMAKE_CURRENT_BINARY_DIR}/resource.res)

add_executable(fisch-macro
src/main.cpp
src/fisch.cpp
src/gui.cpp
)

add_custom_command(TARGET fisch-macro POST_BUILD
  COMMAND "C:/Program Files (x86)/Windows Kits/10/bin/10.0.26100.0/x64/mt.exe" -manifest ${CMAKE_CURRENT_SOURCE_DIR}/src/fisch-macro.exe.manifest -outputresource:$<TARGET_FILE:fisch-macro>;#1
  COMMENT "Embedding manifest into executable"
)

target_compile_options(fisch-macro PRIVATE -Wall -Wextra -Wpedantic)
target_precompile_headers(fisch-macro PRIVATE src/pch.hpp)

target_link_libraries(fisch-macro PRIVATE opencv imgui)
