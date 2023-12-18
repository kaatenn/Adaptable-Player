file(GLOB_RECURSE files "*.cpp" "*.h" "*.hpp" "*.ipp")
list(APPEND total_files ${files})
set(asio2_files ${total_files} PARENT_SCOPE)