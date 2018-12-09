cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(AsyncQueueDir "${CMAKE_SOURCE_DIR}/../AsyncQueue")
link_directories("${AsyncQueueDir}/bin")

file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB ExeSources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB InternalIncludes "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB Includes "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h")
file(GLOB SpecificSources "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}/*.cpp")

if (UNIX)
    add_compile_options(-pthread)
endif(UNIX)

add_library(slaglib ${Sources} ${SpecificSources} ${Includes} ${InternalIncludes})
target_link_libraries(slaglib asyncqueue$<$<CONFIG:Debug>:d>)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/inc" "${AsyncQueueDir}/inc" )
target_include_directories(slaglib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(slag ${ExeSources})
target_link_libraries(slag asyncqueue$<$<CONFIG:Debug>:d> slaglib)

add_dependencies(slag slaglib)

if (WIN32)
    target_link_libraries(slag shlwapi)
elseif(UNIX)
    target_link_libraries(slag dl pthread)
endif(WIN32)

add_custom_command(TARGET slag POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:slag> ${CMAKE_BINARY_DIR}
)

add_test(NAME DryRun COMMAND slag ../Testing/emptygraph.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
 )