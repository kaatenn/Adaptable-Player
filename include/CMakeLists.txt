file(GLOB_RECURSE files "*.cpp" "*.h" "*.hpp")
list(APPEND total_files ${files})
set(inc_files ${total_files} PARENT_SCOPE)