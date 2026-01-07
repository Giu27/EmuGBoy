add_subdirectory(SDL)

add_library(imgui STATIC 
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC
    imgui
    imgui/backends
    imgui_memory_editor
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)