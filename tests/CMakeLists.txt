include(../cmake/GoogleTest.cmake)

add_executable(
    lexer_test
    lexer_test.cc
)

add_executable(
    parser_test
    parser_test.cc
)

add_executable(
    code_test
    code_test.cc
)

add_executable(
    compiler_test
    compiler_test.cc
)

target_link_libraries(
    lexer_test
    GTest::gtest_main
    GTest::gmock_main
    lexer
    token
)

target_link_libraries(
    parser_test
    GTest::gtest_main
    GTest::gmock_main
    lexer
    parser
)

target_link_libraries(
    code_test
    GTest::gtest_main
    GTest::gmock_main
    code
)

target_link_libraries(
    compiler_test
    GTest::gtest_main
    GTest::gmock_main
    compiler
    code
    lexer
    parser
    ast
    object
)

include(GoogleTest)

gtest_discover_tests(lexer_test)
gtest_discover_tests(parser_test)
gtest_discover_tests(code_test)
gtest_discover_tests(compiler_test)
