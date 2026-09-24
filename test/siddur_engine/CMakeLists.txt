add_executable(siddur_engine_tests
  SiddurEngineTest.cpp
  HebrewCalendarTest.cpp
  PrayerContextResolverTest.cpp
  LocalClockTest.cpp
  ServiceGeneratorPlanTest.cpp
  ZmanimTest.cpp
  LiturgicalRulesTest.cpp
  StaticNavigationTest.cpp
)

target_include_directories(siddur_engine_tests PRIVATE
  ${REPO_ROOT}/lib/SiddurEngine
)

target_link_libraries(siddur_engine_tests PRIVATE
  crosspoint_test_common
  GTest::gtest_main
)

gtest_discover_tests(siddur_engine_tests)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(
  NAME siddur_static_navigation_fixture_check
  COMMAND ${Python3_EXECUTABLE}
          ${REPO_ROOT}/scripts/generate_siddur_static_navigation.py
          ${REPO_ROOT}/docs/siddur/APK_STATIC_UI_FIXTURE.json
          ${REPO_ROOT}/lib/SiddurEngine/StaticNavigationData.h
          --check
)
