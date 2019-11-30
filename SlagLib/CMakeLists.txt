cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(AsyncQueueDir "${CMAKE_SOURCE_DIR}/../AsyncQueue")

file(GLOB Sources "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB ExeSources "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB InternalIncludes "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
file(GLOB Includes "${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/inc/slag/*.h")
file(GLOB SpecificSources "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_SYSTEM_NAME}/*.cpp")

if (UNIX)
    add_compile_options(-pthread)
endif(UNIX)

add_library(slaglib ${Sources} ${SpecificSources} ${Includes} ${InternalIncludes})

if (UNIX)
    add_compile_options(-pthread)
    add_library(slaglibpic ${Sources} ${SpecificSources} ${Includes} ${InternalIncludes})
    target_compile_options(slaglibpic PUBLIC "-fpic")
    
    target_include_directories(slaglibpic PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
endif(UNIX)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}/inc" "${AsyncQueueDir}/inc" )
target_include_directories(slaglib PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_executable(slag ${ExeSources})
add_dependencies(slag slaglib)

if (WIN32)
    target_link_libraries(slag slaglib)
    target_link_libraries(slaglib shlwapi)
elseif(UNIX)
    target_link_libraries(slag slaglib)
    target_link_libraries(slaglib dl pthread)
    target_link_libraries(slaglibpic dl pthread)
    add_custom_command(TARGET slaglibpic POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:slaglibpic> ${CMAKE_BINARY_DIR}
    )
endif(WIN32)

add_custom_command(TARGET slag POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:slag> ${CMAKE_BINARY_DIR}
)

add_custom_command(TARGET slaglib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:slaglib> ${CMAKE_BINARY_DIR}
)

add_test(NAME DryRun COMMAND slag ../Testing/emptygraph.cfg
         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)
