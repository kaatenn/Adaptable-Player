file(GLOB_RECURSE files "*.cpp" "*.h" "*.hpp")
list(APPEND total_files ${files})
#[[list(REMOVE_ITEM total_files ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)]]
list(REMOVE_ITEM total_files ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp ${CMAKE_CURRENT_SOURCE_DIR}/test.h)
set(src_files ${total_files} PARENT_SCOPE)