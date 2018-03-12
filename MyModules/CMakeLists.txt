cmake_minimum_required (VERSION 2.8)
project (MyModules)

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_compile_options("-std=c++11")

file(GLOB Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB Includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_library(MyModules SHARED ${Sources} ${Includes})
SET_TARGET_PROPERTIES(MyModules PROPERTIES PREFIX "")

add_custom_command(TARGET MyModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MyModules> ${CMAKE_BINARY_DIR}
)

add_test(NAME AddCpp COMMAND Slag.exe ../Testing/graphAdd.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME Quit COMMAND Slag.exe ../Testing/graphQuit.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
