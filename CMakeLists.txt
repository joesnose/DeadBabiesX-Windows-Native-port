cmake_minimum_required(VERSION 3.20)
project(DeadBabiesXWindowsNative LANGUAGES C CXX)
if(NOT WIN32)
  message(FATAL_ERROR "Use a Windows C++ toolchain, or cross-compile with MinGW-w64.")
endif()
add_subdirectory(source)
add_custom_target(copy_runtime ALL
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:DeadBabiesXNative>"
  DEPENDS DeadBabiesXNative)
