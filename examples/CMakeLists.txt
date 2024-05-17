cmake_minimum_required(VERSION 3.8)

project("examples")

include_directories("../include")

if(MSVC)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CONFIG STREQUAL "Debug")
    add_compile_options(/GL)
    set_directory_properties(PROPERTIES LINK_FLAGS /LTCG:incremental)
  #else()
  #  add_compile_options(-flto=thin)
  endif()

  add_compile_options(/permissive- /W4 /WX)
else()
  #if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  #  add_compile_options(-flto=thin)
  #else()
  #  add_compile_options(-flto)
  #endif()
  add_compile_options(-pedantic-errors 
                      -Wall -Wextra -Wpedantic -Werror
                      -Wno-gnu-zero-variadic-macro-arguments)
endif()

if(MINJSONCPP_USE_TERMINAL_COLOR_SEQUENCES)
  add_compile_definitions(USE_TERMINAL_COLOR_SEQUENCES)
endif()

set(
  CMAKE_CXX_STANDARD 17
  CMAKE_CXX_STANDARD_REQUIRED ON
  CMAKE_CXX_EXTENSIONS OFF
)

add_executable("visit" visit.cpp)
add_executable("resolve" resolve.cpp)
add_executable("escape" escape.cpp)
add_executable("escape_impl" escape_impl.cpp)
add_executable("unescape" unescape.cpp)
add_executable("unescape_impl" unescape_impl.cpp)
add_executable("serialize" serialize.cpp)
add_executable("serialize_impl" serialize_impl.cpp)

add_executable("parse" parse.cpp)
add_custom_command(
  TARGET "parse" POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/"hello_world.json"
  ${CMAKE_CURRENT_BINARY_DIR}/
)

add_executable("parse_impl" parse_impl.cpp)
add_custom_command(
  TARGET "parse_impl" POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/"ndjson.txt"
  ${CMAKE_CURRENT_BINARY_DIR}/
)

add_executable("print_html" print_html.cpp)

if(MINJSONCPP_USE_TERMINAL_COLOR_SEQUENCES)
  add_executable("print_color" print_color.cpp)
endif()