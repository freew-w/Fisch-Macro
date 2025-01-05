add_library(imgui STATIC
imgui_demo.cpp
imgui_draw.cpp
imgui_impl_dx9.cpp
imgui_impl_win32.cpp
imgui_tables.cpp
imgui_widgets.cpp
imgui.cpp
)

target_include_directories(imgui PUBLIC ../../include/)
target_include_directories(imgui PUBLIC ../../include/imgui/)

target_link_directories(imgui INTERFACE "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64/")
target_link_libraries(imgui INTERFACE d3d9 dwmapi)
