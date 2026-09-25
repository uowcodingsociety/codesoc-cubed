cmake_minimum_required(VERSION 3.25)
project(
  codesoc-cubed
  VERSION 1.0.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CODESOC_BUILD_GAME "Build the Raylib game" ON)
option(CODESOC_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

add_library(codesoc_world src/world.cpp)
target_include_directories(codesoc_world PUBLIC src)

if(MSVC)
  target_compile_options(codesoc_world PRIVATE /W4)
else()
  target_compile_options(codesoc_world PRIVATE -Wall -Wextra -Wpedantic)
  if(CODESOC_SANITIZERS)
    target_compile_options(codesoc_world PUBLIC -fsanitize=address,undefined
                                                -fno-omit-frame-pointer)
    target_link_options(codesoc_world PUBLIC -fsanitize=address,undefined)
  endif()
endif()

if(CODESOC_BUILD_GAME)
  include(FetchContent)
  set(BUILD_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)
  set(BUILD_GAMES
      OFF
      CACHE BOOL "" FORCE)
  set(CUSTOMIZE_BUILD
      OFF
      CACHE BOOL "" FORCE)
  FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
    URL_HASH SHA256=2b3ee1e2120c7a0796b33062c7e9a694dd8a8caa56a96319ac8c8ecf54a90d0b
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(raylib)
  # Disable Raylib's F12 shortcut without changing its default frame swapping and input polling.
  target_compile_definitions(raylib PRIVATE SUPPORT_SCREEN_CAPTURE=0)
  add_executable(codesoc-cubed src/main.cpp src/application.cpp src/renderer.cpp)
  target_link_libraries(codesoc-cubed PRIVATE codesoc_world raylib)
  target_include_directories(codesoc-cubed SYSTEM
                             PRIVATE $<TARGET_PROPERTY:raylib,INTERFACE_INCLUDE_DIRECTORIES>)
  if(NOT MSVC)
    target_compile_options(codesoc-cubed PRIVATE -Wall -Wextra -Wpedantic)
  endif()
  install(TARGETS codesoc-cubed RUNTIME DESTINATION bin)
endif()

include(CTest)

if(BUILD_TESTING)
  add_executable(world_tests tests/world_tests.cpp)
  target_link_libraries(world_tests PRIVATE codesoc_world)
  add_test(NAME world_tests COMMAND world_tests)
  if(CODESOC_BUILD_GAME)
    add_executable(renderer_tests tests/renderer_tests.cpp)
    target_link_libraries(renderer_tests PRIVATE codesoc_world raylib)
    target_include_directories(renderer_tests SYSTEM
                               PRIVATE $<TARGET_PROPERTY:raylib,INTERFACE_INCLUDE_DIRECTORIES>)
    add_test(NAME renderer_tests COMMAND renderer_tests)
  endif()
endif()
