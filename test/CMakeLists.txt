cmake_minimum_required(VERSION 3.19)

# Project details
project(
  ${CMAKE_PROJECT_NAME}Tests
  LANGUAGES CXX
)

verbose_message("Adding tests under ${CMAKE_PROJECT_NAME}Tests...")

foreach(source ${test_sources})
  string(REGEX REPLACE "test/" "" file ${source})
  string(REGEX REPLACE "(.*/)([a-zA-Z0-9_ ]+)(\.cpp)" "\\2" test_name ${file})
  add_executable(${test_name}_Tests ${file})

  # Setup code coverage if enabled
  if(${CMAKE_PROJECT_NAME}_ENABLE_CODE_COVERAGE)
    target_compile_options(${CMAKE_PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(${CMAKE_PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
    verbose_message("Code coverage is enabled and provided with GCC.")
  endif()

  # Load the desired unit testing framework
  # Currently supported: GoogleTest (and GoogleMock), Catch2.
  set(${CMAKE_PROJECT_NAME}_TEST_LIB ${PROJECT_LIBRARY})

  if(${CMAKE_PROJECT_NAME}_USE_GTEST)
    find_package(GTest REQUIRED)

    if(${CMAKE_PROJECT_NAME}_USE_GOOGLE_MOCK)
      set(GOOGLE_MOCK_LIBRARIES GTest::gmock GTest::gmock_main)
    endif()

    target_link_libraries(
      ${test_name}_Tests
      PUBLIC
      GTest::GTest
      GTest::Main
      ${GOOGLE_MOCK_LIBRARIES}
      ${${CMAKE_PROJECT_NAME}_TEST_LIB}
    )
  elseif(${CMAKE_PROJECT_NAME}_USE_CATCH2)
    find_package(Catch2 REQUIRED)
    target_link_libraries(
      ${test_name}_Tests
      PUBLIC
      Catch2::Catch2WithMain
      ${${CMAKE_PROJECT_NAME}_TEST_LIB}
    )
  else()
    message(FATAL_ERROR "Unknown testing library. Please setup your desired unit testing library by using `target_link_libraries`.")
  endif()

  # Add the unit tests
  add_test(
    NAME
    ${test_name}
    COMMAND
    ${test_name}_Tests
  )
endforeach()

verbose_message("Finished adding unit tests for ${CMAKE_PROJECT_NAME}.")
