add_executable(apa102c_tests
    test_apa102c.cpp
)

target_link_libraries(apa102c PRIVATE
    GTest::gmock_main
)

include(GoogleTest)
gtest_discover_tests(apa102c_tests)
