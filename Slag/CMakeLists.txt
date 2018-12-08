cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(AsyncQueueDir ${CMAKE_SOURCE_DIR}/../AsyncQueue)
link_directories(${AsyncQueueDir}/bin)

file(GLOB Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB Includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

if (WIN32)
    file(GLOB SourceWin RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} WIN/*.cpp)
    set(Sources ${Sources} ${SourceWin} )
elseif(UNIX)
    file(GLOB SourceUnix RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} UNIX/*.cpp)
    set(Sources ${Sources} ${SourceUnix} )
    add_compile_options(-pthread)
endif(WIN32)

add_library(slaglib ${Sources} ${Includes})

target_link_libraries(slaglib asyncqueue)

include_directories( ${CMAKE_SOURCE_DIR}/inc ${AsyncQueueDir}/inc )

if (WIN32)
    target_link_libraries(slaglib shlwapi)
elseif(UNIX)
    target_link_libraries(slaglib dl pthread)
endif(WIN32)

add_library(slag INTERFACE)
target_link_libraries(slag INTERFACE slaglib)