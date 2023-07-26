include(GoogleTest)

file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUTests ${SRC})

sanatize("address,undefined")

target_compile_definitions(
  MDUTests
  PUBLIC
    $<$<BOOL:${MDU_TESTS_MASTER_KEY}>:MDU_TESTS_MASTER_KEY="${MDU_TESTS_MASTER_KEY}">
)

target_common_warnings(MDUTests PRIVATE)

cpmaddpackage("gh:google/googletest#main")
cpmaddpackage("gh:ericniebler/range-v3#0.12.0")

target_link_libraries(MDUTests PRIVATE MDU::MDU GTest::gtest GTest::gmock
                                       range-v3)

gtest_discover_tests(MDUTests)
