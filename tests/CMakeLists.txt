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
target_common_errors(MDUTests PRIVATE -Werror)

cpmaddpackage(URI "gh:google/googletest#main" OPTIONS "INSTALL_GTEST OFF")
cpmaddpackage("gh:ericniebler/range-v3#0.12.0")

target_link_libraries(MDUTests PRIVATE MDU::MDU GTest::gtest_main GTest::gmock
                                       range-v3)

gtest_discover_tests(MDUTests)
