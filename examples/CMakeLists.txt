file(GLOB_RECURSE SRC *.cpp)
add_executable(MDUExamples ${SRC})

sanatize("address,undefined")

target_common_warnings(MDUExamples)

target_link_libraries(MDUExamples PRIVATE MDU::MDU)
