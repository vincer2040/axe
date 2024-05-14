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

include(GoogleTest)

gtest_discover_tests(lexer_test)
