cmake_minimum_required(VERSION 3.26)
project(adaptable_uploader)

set(CMAKE_CXX_STANDARD 23)

set(PRJ_3RD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd)
set(PRJ_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PRJ_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(PRJ_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB libs ${PRJ_LIB_DIR}/*.lib)
link_libraries(${libs})
include_directories(${PRJ_INC_DIR})

#[[KCP]]
set(KCP_DIR ${PRJ_3RD_DIR}/kcp)
file(GLOB kcp_header ${KCP_DIR}/*.h)
list(APPEND PRJ_EXE_LIST ${kcp_header})
include_directories(${KCP_DIR})

#[[asio]]
set(ASIO_DIR ${PRJ_3RD_DIR}/asio)
include_directories(${ASIO_DIR})
add_subdirectory(${ASIO_DIR})
list(APPEND PRJ_EXE_LIST ${asio_files})

#[[imgui]]
set(IMGUI_DIR ${PRJ_3RD_DIR}/imgui)
include_directories(${IMGUI_DIR})
add_subdirectory(${IMGUI_DIR})
list(APPEND PRJ_EXE_LIST ${imgui_files})

#[[nlohmann]]
set(NLOHMANN_DIR ${PRJ_3RD_DIR}/nlohmann)
include_directories(${NLOHMANN_DIR})
file(GLOB nlohmann_header ${NLOHMANN_DIR}/*.hpp)
list(APPEND PRJ_EXE_LIST ${nlohmann_header})

#[[include]]
add_subdirectory(${PRJ_INC_DIR})
list(APPEND PRJ_EXE_LIST ${inc_files})
include_directories(${PRJ_INC_DIR})

#[[src]]
add_subdirectory(${PRJ_SRC_DIR})
include_directories(${PRJ_SRC_DIR})
list(APPEND PRJ_EXE_LIST ${src_files})

add_executable(adaptable_uploader ${PRJ_EXE_LIST})
