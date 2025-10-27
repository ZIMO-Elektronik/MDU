file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUZppLoad ${SRC})

sanitize(address,undefined)

target_common_warnings(MDUZppLoad PRIVATE)
target_common_errors(MDUZppLoad PRIVATE)

target_link_libraries(MDUZppLoad PRIVATE MDU::MDU)
