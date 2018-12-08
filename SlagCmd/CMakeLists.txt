cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

### requirements
set(AsyncQueueDir ${CMAKE_SOURCE_DIR}/../AsyncQueue)
set(HrDir ${CMAKE_SOURCE_DIR}/../human_readable)
###

file(GLOB Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB Includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

link_directories(${HrDir}/bin ${AsyncQueueDir}/bin)

add_executable(SlagCmd ${Sources} ${Includes})

target_include_directories(SlagCmd PRIVATE
        ${CMAKE_SOURCE_DIR}/inc
        ${CMAKE_SOURCE_DIR}/Slag
        ${AsyncQueueDir}/inc
        ${HrDir} )

target_link_libraries(SlagCmd hrlib slaglib)

add_custom_command(TARGET SlagCmd POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SlagCmd> ${CMAKE_BINARY_DIR}
)

add_test(NAME DryRunCmd COMMAND SlagCmd ../Testing/emptygraph.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
