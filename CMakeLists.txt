cmake_minimum_required (VERSION 3.1)
project (SLAG)

include(CTest)

add_subdirectory(SlagLib)

add_subdirectory(CModules)
add_subdirectory(MyModules)
add_subdirectory(ImgModules)

if (WIN32)
    # add_subdirectory(SlagCmd)
    add_subdirectory(WinSlag)
endif(WIN32)
