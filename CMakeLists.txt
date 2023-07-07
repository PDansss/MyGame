cmake_minimum_required(VERSION 3.5)

project(GAME LANGUAGES CXX C)

set(CMAKE_PREFIX_PATH "D:/Try SDL")
find_package(SDL3 REQUIRED)

file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})

add_library(engine SHARED engine/include/engine.hpp  engine/include/my_audio.hpp engine/include/my_math.hpp
                          engine/include/picopng.hpp engine/src/engine.cpp engine/glad/glad.c
                          engine/glad/glad.h engine/KHR/khrplatform.h engine/ImGUI/imconfig.h engine/ImGUI/imgui.cpp
                          engine/ImGUI/imgui.h engine/ImGUI/imgui_demo.cpp engine/ImGUI/imgui_draw.cpp
                          engine/ImGUI/imgui_widgets.cpp engine/ImGUI/imstb_rectpack.h engine/ImGUI/imgui_internal.h
                          engine/ImGUI/imgui_tables.cpp engine/ImGUI/imstb_textedit.h engine/ImGUI/imstb_truetype.h)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIRECTORY})
target_include_directories(engine PUBLIC engine/include engine/glad engine/KHR engine/ImGUI)
target_compile_features(engine PUBLIC cxx_std_20)
target_link_libraries(engine PRIVATE SDL3::SDL3-shared)

add_executable(game  game/src/game.cpp game/src/tank.cpp game/include/tank.hpp game/src/enemy.cpp
                     game/include/object.hpp game/include/animation.hpp game/include/missile.hpp 
                     game/include/morphing.hpp game/src/animation.cpp game/src/missile.cpp)
target_include_directories(game PUBLIC game/include)
target_compile_features(game PUBLIC cxx_std_20)
target_link_libraries(game PRIVATE engine)

option(OM_ENABLE_ADDRESS_SANITIZER_FOR_SPRITE_TEST
       [[Use address sanitizer to find out memory bug.
Works only on Linux. You should already install library:
```sudo dnf install libasan```
To see line numbers in asan log configure your build with
```CMAKE_BUILD_TYPE=Debug```.]] OFF)

if(OM_ENABLE_ADDRESS_SANITIZER_FOR_SPRITE_TEST)
    message(STATUS "use address sanitizer to find memory bug in GAME")
    add_library(sanitizer INTERFACE)
    target_compile_options(sanitizer INTERFACE -fsanitize=address)
    target_link_options(sanitizer INTERFACE -fsanitize=address)

    target_link_libraries(engine PRIVATE sanitizer)
    target_link_libraries(game PRIVATE sanitizer)
endif()

