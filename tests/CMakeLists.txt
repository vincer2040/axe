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

add_executable(
    vm_test
    vm_test.cc
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

target_link_libraries(
    vm_test
    GTest::gtest_main
    GTest::gmock_main
    compiler
    code
    lexer
    parser
    ast
    object
    vm
)

include(GoogleTest)

gtest_discover_tests(lexer_test)
gtest_discover_tests(parser_test)
gtest_discover_tests(code_test)
gtest_discover_tests(compiler_test)
gtest_discover_tests(vm_test)
