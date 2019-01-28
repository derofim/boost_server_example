
cmake_minimum_required( VERSION 3.13.3 FATAL_ERROR )

set( PROJECT_NAME "boostander" )
set( PROJECT_TARGET_EXE "boostander" )
set( PROJECT_DESCRIPTION "boostander: C++17 boost.beast server" )

set( ROOT_PROJECT_NAME ${PROJECT_NAME} CACHE INTERNAL "ROOT_PROJECT_NAME" )
set( ${PROJECT_NAME}_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "${PROJECT_NAME}_PROJECT_DIR" )

include( cmake/Utils.cmake )

include( cmake/useGoldLinker.cmake )

set_project_version(0 0 1) # from Utils.cmake

project( ${PROJECT_NAME}
  VERSION ${${PROJECT_NAME}_VERSION}
  DESCRIPTION ${PROJECT_DESCRIPTION}
)

set_cmake_module_paths( ${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" ) # from Utils.cmake

print_cmake_system_info() # from Utils.cmake

check_supported_os() # from Utils.cmake

add_subdirectory( submodules )

include( Dependencies.cmake )

include( ProjectFiles.cmake )

add_library( ${PROJECT_NAME}_lib OBJECT
  ${SOURCE_FILES}
  ${${PROJECT_NAME}_OTHER_IDE_FILES_EXTRA}
  )

target_include_directories( ${PROJECT_NAME}_lib PUBLIC "src/" )

target_include_directories( ${PROJECT_NAME}_lib SYSTEM PUBLIC
  ${THIRDPARTY_FILES} ) # from ProjectFiles.cmake

# https://www.boost.org/doc/libs/1_60_0/libs/log/doc/html/log/installation/config.html
set(BOOST_DEFINITIONS "-DBOOST_LOG_DYN_LINK -DBOOST_LOG_USE_NATIVE_SYSLOG" CACHE INTERNAL "" FORCE)

target_compile_definitions( ${PROJECT_NAME}_lib PUBLIC
  ${BOOST_DEFINITIONS} )

add_executable( ${PROJECT_TARGET_EXE} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp )

target_include_directories( ${PROJECT_TARGET_EXE} PUBLIC "src/" )

target_link_libraries(${PROJECT_TARGET_EXE} PRIVATE ${PROJECT_NAME}_lib)

# IWYU detects superfluous includes and when the include can be replaced with a forward declaration.
# It can be obtained using "apt-get install iwyu" or from "github.com/include-what-you-use".
# make sure it can find Clang built-in headers (stdarg.h and friends.)
# see https://stackoverflow.com/a/30951493/10904212
option(ENABLE_IWYU "ENABLE IWYU" OFF)
if (ENABLE_IWYU)
  set(IWYU_IMP "${CMAKE_SOURCE_DIR}/cmake/iwyu/iwyu.imp")
  findPackageCrossPlatform(PythonInterp) # Required by iwyu
  add_iwyu( ${PROJECT_TARGET_EXE} ) # from Utils.cmake
else()
  message(WARNING "iwyu turned off")
endif()

set_source_files_properties( ${OTHER_FILES} PROPERTIES
  HEADER_FILE_ONLY TRUE )

set( USED_BOOST_LIBS
  Boost::boost
  Boost::atomic
  Boost::chrono
  Boost::date_time
  Boost::filesystem
  Boost::graph
  Boost::log
  Boost::program_options
  Boost::regex
  Boost::regex
  Boost::system
  Boost::thread
  CACHE INTERNAL "USED_BOOST_LIBS" )

set( USED_SYSTEM_LIBS
  Threads::Threads # pthread, https://cmake.org/cmake/help/v3.13/module/FindThreads.html
  OpenSSL::SSL # https://cmake.org/cmake/help/v3.10/module/FindOpenSSL.html
  OpenSSL::Crypto # https://cmake.org/cmake/help/v3.10/module/FindOpenSSL.html
  ${X11_LIBRARIES} # https://cmake.org/cmake/help/v3.13/module/FindX11.html
  ${CMAKE_DL_LIBS} # https://cmake.org/cmake/help/v3.13/variable/CMAKE_DL_LIBS.html
  EXPAT::EXPAT # https://cmake.org/cmake/help/v3.13/module/FindEXPAT.html
  ZLIB::ZLIB # https://cmake.org/cmake/help/v3.13/module/FindZLIB.html
  # Custom libs
  stdc++fs # C++17 std::filesystem
  CACHE INTERNAL "USED_SYSTEM_LIBS" )

set( USED_3DPARTY_LIBS
  # boost libs
  ${USED_BOOST_LIBS}
  CACHE INTERNAL "USED_3DPARTY_LIBS" )

set_target_properties( ${PROJECT_NAME}_lib PROPERTIES
  OUTPUT_NAME ${PROJECT_NAME}
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  CMAKE_CXX_STANDARD_REQUIRED ON
  CMAKE_CXX_FLAGS "-fno-rtti /std:c++latest"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${PROJECT_NAME}_lib )

set_target_properties( ${PROJECT_TARGET_EXE} PROPERTIES
  OUTPUT_NAME ${PROJECT_NAME}
  CXX_STANDARD 17
  CXX_EXTENSIONS OFF
  CMAKE_CXX_STANDARD_REQUIRED ON
  CMAKE_CXX_FLAGS "-fno-rtti /std:c++latest"
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/${PROJECT_NAME} )

# @see https://stackoverflow.com/a/46132078/10904212
message( "CMAKE_CXX_COMPILER_ID=${CMAKE_CXX_COMPILER_ID}" )
if( UNIX OR MINGW )
  target_compile_options( ${PROJECT_NAME}_lib PRIVATE
   "$<$<CONFIG:RELEASE>:-Werror>"
    -Wall
    -Wextra
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-deprecated
    -Wno-reorder
    "$<$<CONFIG:RELEASE>:-Wpedantic>"
    "$<$<CONFIG:RELEASE>:-O3>"
    -fdiagnostics-color=always
    "$<$<CONFIG:DEBUG>:-g>"
    "$<$<CONFIG:DEBUG>:-fno-omit-frame-pointer>" # https://github.com/google/sanitizers/wiki/AddressSanitizer#using-addresssanitizer
    )
  message( "detected compiler: UNIX OR MINGW" )
elseif(CMAKE_COMPILER_IS_GNUCXX OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|AppleClang" )
  target_compile_options( ${PROJECT_NAME}_lib PRIVATE
    "$<$<CONFIG:RELEASE>:-Werror>"
    -Wall
    -Wextra
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-deprecated
    -Wno-reorder
    "$<$<CONFIG:RELEASE>:-pedantic>"
    "$<$<CONFIG:RELEASE>:-O3>"
    )
  message( "detected compiler: Clang" )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
  target_compile_options( ${PROJECT_NAME}_lib PRIVATE /W4 /WX )
else()
  message( FATAL_ERROR "unsupported compiler" )
endif()

target_link_libraries( ${PROJECT_NAME}_lib PUBLIC
  # 3dparty libs
  ${USED_3DPARTY_LIBS}
  # system libs
  ${USED_SYSTEM_LIBS}
)

add_custom_command( TARGET ${PROJECT_TARGET_EXE} PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:${PROJECT_TARGET_EXE}>/assets )

option(BUILD_TESTS "Enable tests" ON)
if(BUILD_TESTS)
  add_subdirectory( tests )
endif()

add_subdirectory( client )
