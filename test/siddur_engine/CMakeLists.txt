add_executable(siddur_engine_tests
  SiddurEngineTest.cpp
)

target_link_libraries(siddur_engine_tests PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(siddur_engine_tests)
