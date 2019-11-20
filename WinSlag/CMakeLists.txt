cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

add_compile_options( "/DNOMINMAX" )

set(AsyncQueueDir "${CMAKE_SOURCE_DIR}/../AsyncQueue")

file(GLOB Sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp)
file(GLOB Includes RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.h)

add_executable(WinSlag ${Sources} ${Includes})

target_include_directories(WinSlag PRIVATE
        "${CMAKE_SOURCE_DIR}/SlagLib/inc"
        "${AsyncQueueDir}/inc"
        )

add_dependencies(WinSlag slaglib)

target_link_libraries(WinSlag slaglib shlwapi comctl32)

add_custom_command(TARGET WinSlag POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:WinSlag> "${CMAKE_BINARY_DIR}"
)

add_test(NAME WinSlag1 COMMAND WinSlag ../Testing/ImgProcessor1.cfg
         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
