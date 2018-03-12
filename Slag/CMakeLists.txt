cmake_minimum_required (VERSION 2.8)

### requirements
set(AsyncQueueDir ${CMAKE_SOURCE_DIR}/../AsyncQueue)
set(HrDir ${CMAKE_SOURCE_DIR}/../human_readable)
###

file(GLOB Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB Includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

if (WIN32)
    file(GLOB SourceWin RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} WIN/*.cpp)
    set(Sources ${Sources} ${SourceWin} )
elseif(UNIX)
    file(GLOB SourceUnix RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} UNIX/*.cpp)
    set(Sources ${Sources} ${SourceUnix} )
    
    add_compile_options("-std=c++11" -pthread)
endif(WIN32)

link_directories(${AsyncQueueDir}/bin ${HrDir}/bin ${OPENCV_LIB})

add_executable(Slag ${Sources} ${Includes})
SET_TARGET_PROPERTIES(Slag PROPERTIES SUFFIX ".exe")

target_include_directories(Slag PRIVATE
        ${CMAKE_SOURCE_DIR}/inc
        ${AsyncQueueDir}/inc
        ${HrDir} )

target_link_libraries(Slag asyncqueue hrlib)

if (WIN32)
    target_link_libraries(Slag shlwapi)
elseif(UNIX)
    target_link_libraries(Slag dl pthread)
endif(WIN32)

add_custom_command(TARGET Slag POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Slag> ${CMAKE_BINARY_DIR}
)

add_test(NAME DryRun COMMAND Slag.exe ../Testing/emptygraph.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
