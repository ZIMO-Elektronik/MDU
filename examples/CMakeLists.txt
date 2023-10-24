file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUExamples ${SRC})

sanitize("address,undefined")

target_common_warnings(MDUExamples PRIVATE)

target_link_libraries(MDUExamples PRIVATE MDU::MDU)
