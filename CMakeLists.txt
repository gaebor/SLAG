cmake_minimum_required (VERSION 2.8)
project (SLAG)

set(AsyncQueueDir ${CMAKE_SOURCE_DIR}/../AsyncQueue)
set(HrDir ${CMAKE_SOURCE_DIR}/../human_readable)

set(OPENCV_VERSION 341)

file(GLOB SlagSource RELATIVE ${CMAKE_SOURCE_DIR} Slag/*.cpp)

if (WIN32)
    file(GLOB SlagSourceWin RELATIVE ${CMAKE_SOURCE_DIR} Slag/WIN/*.cpp)
    set(SlagSource ${SlagSource} ${SlagSourceWin} )
    set(OPENCV_DIR "E:/PROGRAMOK/opencv/build")
    set(OPENCV_VER "${OPENCV_DIR}/x64/vc15")
    set(OPENCV_BIN "${OPENCV_VER}/bin")
    set(OPENCV_LIB "${OPENCV_VER}/lib")
    add_compile_options("/DNDEBUG")
elseif(UNIX)
    file(GLOB SlagSourceUnix RELATIVE ${CMAKE_SOURCE_DIR} Slag/UNIX/*.cpp)
    set(SlagSource ${SlagSource} ${SlagSourceUnix} )
    
    add_compile_options("-std=c++11" -pthread)
endif(WIN32)

link_directories(${AsyncQueueDir}/bin ${HrDir}/bin ${OPENCV_LIB})

### Slag ###
file(GLOB SlagInclude RELATIVE ${CMAKE_SOURCE_DIR} Slag/*.h)

add_executable(Slag ${SlagSource} ${SlagInclude})

include_directories(Slag inc)
target_include_directories(Slag PRIVATE ${AsyncQueueDir}/inc ${HrDir})
include_directories(Slag "${OPENCV_DIR}/include")    

target_link_libraries(Slag asyncqueue hrlib)

if (WIN32)
    target_link_libraries(Slag shlwapi)
elseif(UNIX)
    target_link_libraries(Slag dl pthread)
endif(WIN32)

add_custom_command(TARGET Slag POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Slag> .
)

### C modules ###
add_library(CModules SHARED CModules/CModules.c)
add_custom_command(TARGET CModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:CModules> .
)
### C++ modules ###
file(GLOB MyModulesSource RELATIVE ${CMAKE_SOURCE_DIR} MyModules/*.cpp)
file(GLOB MyModulesInclude RELATIVE ${CMAKE_SOURCE_DIR} MyModules/*.h)
add_library(MyModules SHARED ${MyModulesSource} ${MyModulesInclude})

set(OpenCvDep opencv_world${OPENCV_VERSION})
target_link_libraries(MyModules ${OpenCvDep})

add_custom_command(TARGET MyModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MyModules> .
)

add_custom_command(TARGET MyModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_BIN}/${OpenCvDep}.dll" .
)

include(CTest)

add_test(DryRun Slag ../Testing/emptygraph.cfg)

add_test(AddText Slag ../Testing/graphCAdd.cfg)
