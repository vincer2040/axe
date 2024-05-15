cmake_minimum_required(VERSION 3.10)

project(axe VERSION 0.0.1)


enable_testing()
add_subdirectory(tests)

set(CMAKE_C_COMPILER "clang++")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -pedantic -fstack-clash-protection \
-fstack-protector-all -fstack-protector-strong -Werror=format-security -Werror=implicit-function-declaration -pipe -O2")

set(CMAKE_C_FLAGS_RELEASE "-Wall -Werror -pedantic -fstack-clash-protection -fstack-protector-all \
-fstack-protector-strong -Werror=format-security -Werror=implicit-function-declaration -pipe -O2 -s -DNDEBUG")

add_library(
    token
    src/token.cc
)

add_library(
    lexer
    src/lexer.cc
)

add_library(
    parser
    src/parser.cc
)

add_library(
    ast
    src/ast.cc
)

add_library(
    object
    src/object.cc
)

add_library(
    code
    src/code.cc
)

add_library(
    compiler
    src/compiler.cc
)

add_executable(
    axec
    src/main.cc
)

target_link_libraries(
    axec
    lexer
    parser
    ast
)

target_link_libraries(
    lexer
    token
)

target_link_libraries(
    parser
    lexer
    token
    ast
)
