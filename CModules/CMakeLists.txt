cmake_minimum_required (VERSION 2.8)
project (CModules)

add_library(CModules SHARED CModules.c)
SET_TARGET_PROPERTIES(CModules PROPERTIES PREFIX "")

include_directories(${CMAKE_SOURCE_DIR}/inc)

add_custom_command(TARGET CModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CModules> ${CMAKE_BINARY_DIR}
)

add_test(NAME AddC COMMAND slag ../Testing/graphCAdd.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
