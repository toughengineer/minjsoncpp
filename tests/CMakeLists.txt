cmake_minimum_required(VERSION 3.8)

set(TARGET_NAME "test")

set(HEADERS
  "utils.h"
)

set(SOURCES
  "Catch2/catch_amalgamated.cpp"
  "compile_time_checks.cpp"
  "value.cpp"
  "visit.cpp"
  "escape.cpp"
  "serialize.cpp"
  "unescape.cpp"
  "parse.cpp"
  "allocator.cpp"
  "json.org_suite.cpp"
  "benchmarks.cpp"
)

set(TEST_DATA
  "canada.json"
  "citm_catalog.json"
)

set(
  CMAKE_CXX_STANDARD 17
  CMAKE_CXX_STANDARD_REQUIRED ON
  CMAKE_CXX_EXTENSIONS OFF
)

add_executable(${TARGET_NAME} ${SOURCES} ${HEADERS})
target_include_directories(${TARGET_NAME} PUBLIC "../include")

set_target_properties(${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(MSVC)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND NOT CONFIG STREQUAL "Debug")
    target_compile_options(${TARGET_NAME} PRIVATE /GL)
    set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS /LTCG:incremental)
  #else()
  #  target_compile_options(${TARGET_NAME} PRIVATE -flto=thin)
  endif()

  target_compile_options(${TARGET_NAME} PRIVATE /permissive- /W4 /WX)
else()
  #if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  #  target_compile_options(${TARGET_NAME} PRIVATE -flto=thin)
  #else()
  #  target_compile_options(${TARGET_NAME} PRIVATE -flto)
  #endif()
  target_compile_options(${TARGET_NAME} PRIVATE -pedantic-errors
                                                -Wall -Wextra -Wpedantic -Werror
                                                -Wno-gnu-zero-variadic-macro-arguments)
endif()

foreach(file ${TEST_DATA})
  set(src ${CMAKE_CURRENT_SOURCE_DIR}/${file})
  set(dst ${CMAKE_BINARY_DIR}/)
  add_custom_command(
    TARGET ${TARGET_NAME} POST_BUILD
    COMMENT Copying ${src} to ${dst}
    COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
  )
endforeach()
