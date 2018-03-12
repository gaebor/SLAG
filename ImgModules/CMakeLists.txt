cmake_minimum_required (VERSION 2.8)
project (ImgModules)

set(OPENCV_VERSION 341)

if (WIN32)
    set(OPENCV_DIR "E:/PROGRAMOK/opencv/build")
    set(OPENCV_VER "${OPENCV_DIR}/x64/vc15")
    set(OPENCV_BIN "${OPENCV_VER}/bin")
    set(OPENCV_LIB "${OPENCV_VER}/lib")
    set(OPENCV_INC "${OPENCV_DIR}/include")
    
    set(OpenCvDep opencv_world${OPENCV_VERSION})
    
    link_directories(${OPENCV_LIB})
elseif(UNIX)
    add_compile_options("-std=c++11")
endif(WIN32)

file(GLOB Sources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB Includes RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_library(ImgModules SHARED ${Sources} ${Includes})
SET_TARGET_PROPERTIES(ImgModules PROPERTIES PREFIX "")

target_link_libraries(ImgModules ${OpenCvDep})

target_include_directories(ImgModules PRIVATE ${CMAKE_SOURCE_DIR}/inc )

if (WIN32)
    target_include_directories(ImgModules PRIVATE 
        ${CMAKE_SOURCE_DIR}/inc "${OPENCV_INC}" )
    add_custom_command(TARGET ImgModules POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${OPENCV_BIN}/${OpenCvDep}.dll" ${CMAKE_BINARY_DIR}
    )
endif(WIN32)

add_custom_command(TARGET ImgModules POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ImgModules> ${CMAKE_BINARY_DIR}
)

add_test(NAME WebCam COMMAND Slag.exe ../Testing/WebCam.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
add_test(NAME ImgProcessor1 COMMAND Slag.exe ../Testing/ImgProcessor1.cfg
         WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

# that's an infinite test
# add_test(NAME ImgProcessor2 COMMAND Slag.exe ../Testing/ImgProcessor2.cfg
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
