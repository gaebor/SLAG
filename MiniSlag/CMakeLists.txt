cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(AsyncQueueDir "${CMAKE_SOURCE_DIR}/../AsyncQueue")
set(SlagLibDir "${CMAKE_SOURCE_DIR}/SlagLib")

set(MiniwebDir "${CMAKE_SOURCE_DIR}/../miniweb-1")

if (UNIX)
    add_compile_options("-pthread")
else()
    add_compile_options("/DNOMINMAX")
endif(UNIX)

add_library(MiniSlag SHARED "${CMAKE_CURRENT_SOURCE_DIR}/MiniSlag.cpp")

# if (UNIX)
    # target_link_libraries(MiniSlag slaglibpic)
# else()
    # target_link_libraries(MiniSlag slaglib)
# endif(UNIX)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${AsyncQueueDir}/inc" "${MiniwebDir}" "${SlagLibDir}/inc")

add_dependencies(MiniSlag slaglib)

if (WIN32)
    target_link_libraries(MiniSlag shlwapi slaglib)
elseif(UNIX)
    target_link_libraries(MiniSlag dl pthread slaglibpic)
endif(WIN32)
