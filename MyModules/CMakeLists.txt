cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

project (MyModules)

include_directories("${CMAKE_SOURCE_DIR}/SlagLib/inc")

file(GLOB Sources RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.cpp)
file(GLOB Includes RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" *.h)

add_library(MyModules SHARED ${Sources} ${Includes})
SET_TARGET_PROPERTIES(MyModules PROPERTIES PREFIX "")

add_custom_command(TARGET MyModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MyModules> "${CMAKE_BINARY_DIR}"
)

add_test(NAME AddCpp COMMAND slag ../Testing/graphAdd.cfg
         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
add_test(NAME Quit COMMAND slag ../Testing/graphQuit.cfg
         WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
