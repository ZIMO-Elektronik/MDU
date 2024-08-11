include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUTests ${SRC})

sanitize(address,undefined)

target_compile_definitions(
  MDUTests
  PUBLIC
    $<$<BOOL:${MDU_TESTS_MASTER_KEY}>:MDU_TESTS_MASTER_KEY="${MDU_TESTS_MASTER_KEY}">
)

target_common_warnings(MDUTests PRIVATE)

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  "google/googletest"
  GIT_TAG
  main
  SYSTEM
  YES
  OPTIONS
  "INSTALL_GTEST OFF")
cpmaddpackage("gh:ericniebler/range-v3#0.12.0")

target_link_libraries(MDUTests PRIVATE MDU::MDU GTest::gtest_main GTest::gmock
                                       range-v3)

gtest_discover_tests(MDUTests)
