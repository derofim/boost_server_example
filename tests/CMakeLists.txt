
cmake_minimum_required( VERSION 3.13.3 FATAL_ERROR )

set( TESTS_PROJECT_NAME "${ROOT_PROJECT_NAME}_tests" )

# Get CMAKE_MODULE_PATH from parent project
list(APPEND CMAKE_MODULE_PATH "${${ROOT_PROJECT_NAME}_CMAKE_MODULE_PATH}")

option(ENABLE_CODE_COVERAGE "ENABLE CODE COVERAGE" OFF)
if (ENABLE_CODE_COVERAGE)
  set( CATCH2_TEST_SETTINGS -r junit -o unit_test_result.xml )
else()
  set( CATCH2_TEST_SETTINGS -r console --use-colour yes --order rand --durations yes --warn NoTests --invisibles)
endif()

include( testRunner ) # start tests as CMake targets

macro(add_to_tests_list source_list)
  list(APPEND UNIT_TEST_SOURCE_LIST ${source_list})
endmacro()

macro(set_test_compile_options target)
  target_compile_options(${target} PRIVATE
              $<$<CXX_COMPILER_ID:MSVC>:
              /W3 # Set warning level
              #/WX # Treats all compiler warnings as errors.
              >
              $<$<CXX_COMPILER_ID:GNU>:
              -Wformat=2
              -Wall
              -W
              "$<$<CONFIG:RELEASE>:-Werror>" # Treats all compiler warnings as errors.
              -Wpedantic
              -fno-omit-frame-pointer # https://github.com/google/sanitizers/wiki/AddressSanitizer#using-addresssanitizer
              >
              $<$<CXX_COMPILER_ID:Clang>:
              -Wformat=2
              -Wall
              -W
              "$<$<CONFIG:RELEASE>:-Werror>" # Treats all compiler warnings as errors.
              -Wpedantic
              -Wdeprecated-register
              -fno-omit-frame-pointer # https://github.com/google/sanitizers/wiki/AddressSanitizer#using-addresssanitizer
              >
          )

  target_compile_definitions( ${target} PRIVATE
    ${BOOST_DEFINITIONS} )

  # @see https://github.com/eranpeer/FakeIt
  target_compile_options( ${target} PRIVATE
    # If you don't use /ZI, you will have exceptions mocking destructors (which includes unique_ptr and other smart pointers).
    $<$<CXX_COMPILER_ID:MSVC>:/ZI>
    # On GCC, optimization flag O2 and O3 are not supported. You must compile the test project with -O1 or -O0.
    $<$<CXX_COMPILER_ID:GNU>:-O0>
    $<$<CXX_COMPILER_ID:Clang>:-O0> )

  set_target_properties( ${target} PROPERTIES
    CXX_STANDARD 17
    CXX_EXTENSIONS OFF
    CMAKE_CXX_STANDARD_REQUIRED ON
    CMAKE_CXX_FLAGS "-fno-rtti /std:c++latest"
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests )
endmacro()

macro(tests_add_executable target source_list)
  list(APPEND UNIT_TEST_TARGETS ${target})

  add_to_tests_list(${source_list})

  add_executable(${target} ${source_list})

  target_link_libraries(${target} PRIVATE test_main)

  target_link_libraries(${target} PRIVATE
    # 3dparty libs
    ${USED_3DPARTY_LIBS}
    # system libs
    ${USED_SYSTEM_LIBS}
    # main project lib
    ${ROOT_PROJECT_NAME}_lib
  )

  set_test_compile_options( ${target} )

  add_test(
    NAME ${target}
    COMMAND ${target} ${CATCH2_TEST_SETTINGS})

  add_test_command_targets( ${target} )

  if(AUTORUN_TESTS)
    add_test_autostarter( ${target} )
  endif()
endmacro()

add_library( test_main OBJECT
  main.cpp
  #${${ROOT_PROJECT_NAME}_SRCS} # all source files  of root project without main.cpp
  #${THIRDPARTY_SOURCES}
  testsCommon.h # include in IDE
  )

# ensure that dependencies build before <target> does.
add_dependencies(test_main ${ROOT_PROJECT_NAME})

set_test_compile_options( test_main )

target_link_libraries(test_main PUBLIC
  # 3dparty libs
  ${USED_3DPARTY_LIBS}
  # system libs
  ${USED_SYSTEM_LIBS}
  # main project lib
  ${ROOT_PROJECT_NAME}_lib
)

target_link_libraries( test_main PUBLIC
  Catch FakeIt )

target_include_directories( test_main SYSTEM PUBLIC
  ${${ROOT_PROJECT_NAME}_PROJECT_DIR}/src
  ${THIRDPARTY_FILES})

option(AUTORUN_TESTS "Autorun tests" OFF) # see add_test_autostarter

option(BUILD_TESTS "Enable tests" ON)
if(BUILD_TESTS)
  message( "${TESTS_PROJECT_NAME} testing enabled" )

  set ( server_deps
    server.test.cpp
)
  tests_add_executable(server "${server_deps}")

#  set ( utils_deps
#    utils.test.cpp
#)
#  tests_add_executable(utils "${utils_deps}")
endif()

#add_to_tests_list(utils)
#add_to_tests_list(server)

tests_add_executable(check_all ${UNIT_TEST_SOURCE_LIST})
#message(FATAL_ERROR ${UNIT_TEST_SOURCE_LIST})

# "server" is one of test program names
add_custom_command( TARGET server POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_SOURCE_DIR}/tests/data ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/tests/data )

enable_testing()

# Run ALL tests with verbose output
# Usage: cmake --build build --target run_all_tests
add_custom_target(run_all_tests
    DEPENDS ${ALL_TESTS_LIST_VERBOSE}
)
foreach(target IN LISTS ALL_TESTS_LIST_VERBOSE)
  add_dependencies(run_all_tests ${target})
endforeach()

if (ENABLE_CODE_COVERAGE)
  set(LCOV_REMOVE_EXTRA "'${${${ROOT_PROJECT_NAME}_PROJECT_DIR}}/submodules/*'" "'test/*'" "'ext/*'" "'abseil/*'" "'submodules/*'" "'*/tests/*'" "'scripts/*'" "'lib/*'" "'build/*'" "'bin/*'" "'*Catch/*'" "'*FakeIt/*'")
  set(CMAKE_BUILD_TYPE Coverage)
  # test coverage build configuration for C++: debug build, no optimization, profiling enabled
  set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
  # include the cmake code coverage module
  include( CodeCoverage )
  # cmake code coverage module target
  setup_target_for_coverage(
    # Name for custom target.
    unit_tests_coverage
    # Name of the test driver executable that runs the tests.
    # NOTE! This should always have a ZERO as exit code
    # otherwise the coverage generation will not complete.
    check_all
    coverage # Name of output directory.
    )
  message("added make code coverage for ${TESTS_PROJECT_NAME}_coverage")
else()
  message(WARNING "code coverage off")
endif()
