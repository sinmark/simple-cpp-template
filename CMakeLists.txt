cmake_minimum_required(VERSION 3.19)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# Project details
project(
  "Project"
  VERSION 0.1.0
  LANGUAGES CXX
)

# Set project options
include(cmake/StandardSettings.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Utils.cmake)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

message(STATUS "Started CMake for ${PROJECT_NAME} v${PROJECT_VERSION}...\n")

if(UNIX)
  add_compile_options("$<$<CONFIG:DEBUG>:-D_DEBUG>") # this will allow to use same _DEBUG macro available in both Linux as well as Windows - MSCV environment. Easy to put Debug specific code.
endif(UNIX)

# Setup alternative names
if(${PROJECT_NAME}_USE_ALT_NAMES)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWERCASE)
  string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)
else()
  set(PROJECT_NAME_LOWERCASE ${PROJECT_NAME})
  set(PROJECT_NAME_UPPERCASE ${PROJECT_NAME})
endif()

# Prevent building in the source directory
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.\n")
endif()

#
# Create library, setup header and source files
#

# Find all headers and implementation files
include(cmake/SourcesAndHeaders.cmake)

if(${PROJECT_NAME}_BUILD_EXECUTABLE)
  add_executable(${PROJECT_NAME} ${exe_sources})

  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
    verbose_message("Found the following sources:")

    foreach(source IN LISTS exe_sources)
      verbose_message("* ${source}")
    endforeach()
  endif()

  if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
    add_library(${PROJECT_NAME}_LIB INTERFACE)
  else()
    add_library(${PROJECT_NAME}_LIB ${headers} ${sources})
  endif()

  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
    verbose_message("Found the following headers:")

    foreach(header IN LISTS headers)
      verbose_message("* ${header}")
    endforeach()
  endif()

  target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_LIB)
elseif(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
  add_library(${PROJECT_NAME} INTERFACE)

  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
    verbose_message("Found the following headers:")

    foreach(header IN LISTS headers)
      verbose_message("* ${header}")
    endforeach()
  endif()
else()
  add_library(
    ${PROJECT_NAME}
    ${headers}
    ${sources}
  )

  if(${PROJECT_NAME}_VERBOSE_OUTPUT)
    verbose_message("Found the following sources:")

    foreach(source IN LISTS sources)
      verbose_message("* ${source}")
    endforeach()

    verbose_message("Found the following headers:")

    foreach(header IN LISTS headers)
      verbose_message("* ${header}")
    endforeach()
  endif()
endif()

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}"
)

if(${PROJECT_NAME}_BUILD_EXECUTABLE)
  set_target_properties(
    ${PROJECT_NAME}_LIB
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}"
    OUTPUT_NAME ${PROJECT_NAME}
  )
endif()

# Project library reference
if(${PROJECT_NAME}_BUILD_EXECUTABLE)
  set(PROJECT_LIBRARY ${PROJECT_NAME}_LIB)
else()
  set(PROJECT_LIBRARY ${PROJECT_NAME})
endif()

verbose_message("Project library reference: ${PROJECT_LIBRARY}\n")

include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_LIBRARY})

verbose_message("Applied compiler warnings. Using standard ${CMAKE_CXX_STANDARD}.\n")

#
# Model project dependencies
#

# Identify and link with the specific "packages" the project uses
# find_package(package_name package_version REQUIRED package_type [other_options])
# target_link_libraries(
# ${PROJECT_NAME}
# PUBLIC
# dependency1 ...
# PRIVATE
# dependency2 ...
# ${PROJECT_NAME}_PROJECT_OPTIONS
# ${PROJECT_NAME}_PROJECT_WARNINGS
# )
# if(${PROJECT_NAME}_BUILD_EXECUTABLE AND ${PROJECT_NAME}_ENABLE_UNIT_TESTING)
# target_link_libraries(
# ${PROJECT_NAME}_LIB
# PUBLIC
# dependency1 ...
# )
# endif()

# For Windows, it is necessary to link with the MultiThreaded library.
# Depending on how the rest of the project's dependencies are linked, it might be necessary
# to change the line to statically link with the library.
#
# This is done as follows:
#
# set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
#
# On Linux and Mac this variable is ignored. If any issues rise from it, try commenting it out
# and letting CMake decide how to link with it.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

verbose_message("Successfully added all dependencies and linked against them.")

#
# Set the build/user include directories
#

# Allow usage of header files in the `src` directory, but only for utilities
if(${PROJECT_NAME}_BUILD_HEADERS_ONLY)
  target_include_directories(
    ${PROJECT_LIBRARY}
    INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
else()
  target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  if(${PROJECT_NAME}_BUILD_EXECUTABLE)
    target_include_directories(
      ${PROJECT_NAME}_LIB
      PUBLIC
      $<INSTALL_INTERFACE:include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
  endif()
endif()

message(STATUS "Finished setting up include directories.")

# Provide alias
if(${PROJECT_NAME}_BUILD_EXECUTABLE)
  add_executable(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
else()
  add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
endif()

verbose_message("Project is now aliased as ${PROJECT_NAME}::${PROJECT_NAME}.\n")

# Format the project using the `clang-format` target (i.e: cmake --build build --target clang-format)
add_clang_format_target()

# Unit testing setup
if(${PROJECT_NAME}_ENABLE_UNIT_TESTING)
  enable_testing()
  message(STATUS "Build unit tests for the project. Tests should always be found in the test folder\n")
  add_subdirectory(test)
endif()

# Put compile_commands.json into project root
if(CMAKE_EXPORT_COMPILE_COMMANDS)
  add_custom_target(
    copy-compile-commands ALL
    ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_LIST_DIR}
  )
endif()
