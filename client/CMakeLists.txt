
cmake_minimum_required( VERSION 3.13.3 FATAL_ERROR )

set( CLIENT_PROJECT_NAME "${ROOT_PROJECT_NAME}_client" )

# Get CMAKE_MODULE_PATH from parent project
list(APPEND CMAKE_MODULE_PATH "${${ROOT_PROJECT_NAME}_CMAKE_MODULE_PATH}")

macro(set_compile_options target)
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
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/client )
endmacro()

set ( CLIENT_SOURCE_LIST
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  #${${ROOT_PROJECT_NAME}_SRCS} # all source files  of root project without main.cpp
  #${THIRDPARTY_SOURCES}
  )

#message(FATAL_ERROR ${CLIENT_SOURCE_LIST})
add_executable(${CLIENT_PROJECT_NAME} ${CLIENT_SOURCE_LIST})

add_dependencies(${CLIENT_PROJECT_NAME} ${ROOT_PROJECT_NAME})

# ensure that dependencies build before <target> does.
#add_dependencies(${CLIENT_PROJECT_NAME} ${ROOT_PROJECT_NAME})

set_compile_options( ${CLIENT_PROJECT_NAME} )

target_link_libraries(${CLIENT_PROJECT_NAME} PRIVATE
  # 3dparty libs
  ${USED_3DPARTY_LIBS}
  # system libs
  ${USED_SYSTEM_LIBS}
  # main project lib
  ${ROOT_PROJECT_NAME}_lib
)

target_include_directories( ${CLIENT_PROJECT_NAME} SYSTEM PRIVATE
  ${${ROOT_PROJECT_NAME}_PROJECT_DIR}/src
  ${THIRDPARTY_FILES} )

# "server" is one of test program names
add_custom_command( TARGET ${CLIENT_PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_SOURCE_DIR}/client/data ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/client/data )

