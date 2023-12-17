cmake_minimum_required(VERSION 3.26)
project(adaptable_uploader)

set(CMAKE_CXX_STANDARD 23)

set(PRJ_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(PRJ_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)
set(PRJ_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB libs ${PRJ_LIB_DIR}/*.lib)
link_libraries(${libs})
include_directories(${PRJ_INC_DIR})

#[[KCP]]
set(KCP_DIR ${PRJ_INC_DIR}/kcp)
file(GLOB kcp_header ${KCP_DIR}/*.h)
list(APPEND PRJ_EXE_LIST ${kcp_header})
include_directories(${KCP_DIR})

#[[asio]]
set(ASIO_DIR ${PRJ_INC_DIR}/asio)
include_directories(${ASIO_DIR})
add_subdirectory(${ASIO_DIR})
list(APPEND PRJ_EXE_LIST ${asio_files})

#[[opengl]]
set(OPENGL_DIR ${PRJ_INC_DIR}/opengl)
include_directories(${OPENGL_DIR})
file(GLOB opengl_header ${OPENGL_DIR}/*.h)
file(GLOB opengl_src ${OPENGL_DIR}/*.c)
list(APPEND PRJ_EXE_LIST ${opengl_header} ${opengl_src})

#[[imgui]]
set(IMGUI_DIR ${PRJ_INC_DIR}/imgui)
file(GLOB imgui_header ${IMGUI_DIR}/*.h)
file(GLOB imgui_cpp ${IMGUI_DIR}/*.cpp)
include_directories(${IMGUI_DIR})
list(APPEND PRJ_EXE_LIST ${imgui_header} ${imgui_cpp})


#[[src]]
add_subdirectory(${PRJ_SRC_DIR})
include_directories(${PRJ_SRC_DIR})
list(APPEND PRJ_EXE_LIST ${src_files})

add_executable(adaptable_uploader ${PRJ_EXE_LIST})
