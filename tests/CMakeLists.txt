include(../cmake/GoogleTest.cmake)

add_executable(
    lexer_test
    lexer_test.cc
)

target_link_libraries(
    lexer_test
    GTest::gtest_main
    GTest::gmock_main
    lexer
    token
)

add_executable(
    parser_test
    parser_test.cc
)

target_link_libraries(
    parser_test
    GTest::gtest_main
    GTest::gmock_main
    lexer
    parser
)

include(GoogleTest)

gtest_discover_tests(lexer_test)
gtest_discover_tests(parser_test)
