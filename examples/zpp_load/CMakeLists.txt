file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUZppLoad ${SRC})

sanitize(address,undefined)

target_link_libraries(MDUZppLoad PRIVATE MDU::MDU)
