cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)

set(AsyncQueueDir "${CMAKE_SOURCE_DIR}/../AsyncQueue")
set(SlagLibDir "${CMAKE_SOURCE_DIR}/SlagLib")
link_directories("${AsyncQueueDir}/bin")

set(MiniwebDir "${CMAKE_SOURCE_DIR}/../miniweb-1")

if (UNIX)
    add_compile_options(-pthread)
endif(UNIX)

add_library(MiniSlag SHARED ${CMAKE_CURRENT_SOURCE_DIR}/MiniSlag.cpp)
target_link_libraries(MiniSlag asyncqueue$<$<CONFIG:Debug>:d> slaglib)

include_directories( "${CMAKE_CURRENT_SOURCE_DIR}" "${AsyncQueueDir}/inc" "${MiniwebDir}" "${SlagLibDir}/inc")

add_dependencies(MiniSlag slaglib)

if (WIN32)
    target_link_libraries(MiniSlag shlwapi)
elseif(UNIX)
    target_link_libraries(MiniSlag dl pthread)
endif(WIN32)
