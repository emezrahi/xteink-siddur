add_executable(siddur_engine_tests
  SiddurEngineTest.cpp
  HebrewCalendarTest.cpp
  PrayerContextResolverTest.cpp
  LocalClockTest.cpp
  ServiceGeneratorPlanTest.cpp
  ZmanimTest.cpp
  LiturgicalRulesTest.cpp
)

target_include_directories(siddur_engine_tests PRIVATE
  ${REPO_ROOT}/lib/SiddurEngine
)

target_link_libraries(siddur_engine_tests PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(siddur_engine_tests)
