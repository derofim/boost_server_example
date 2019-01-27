# Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
# Distributed under the MIT License.
# See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT

set( BOOST_VERSION 1.69.0 )

# If you have built boost statically you will need to set the Boost_USE_STATIC_LIBS CMake variable to ON
# also don`t forget to set DBOOST_LOG_DYN_LINK https://stackoverflow.com/a/17868918/10904212
# set( Boost_USE_STATIC_LIBS FALSE )
# set( Boost_USE_STATIC_RUNTIME FALSE )
# set( Boost_USE_MULTITHREADED TRUE )
set( BOOST_ROOT CACHE STRING /usr )
set( Boost_ADDITIONAL_VERSIONS "${BOOST_VERSION}" )
set( BOOST_LIBS CACHE STRING ${BOOST_ROOT}/lib )
# set( Boost_COMPILER "-gcc" )
findPackageCrossPlatform( Boost 1.69.0
  COMPONENTS program_options filesystem regex date_time system thread graph log
  EXACT REQUIRED )

findPackageCrossPlatform( Threads REQUIRED )
message(STATUS "CMAKE_THREAD_LIBS_INIT = ${CMAKE_THREAD_LIBS_INIT}")

findPackageCrossPlatform( X11 REQUIRED )
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

findPackageCrossPlatform(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
set(OPENSSL_USE_STATIC_LIBS TRUE)
else()
message(FATAL_ERROR "OpenSSL Package not found")
endif()

findPackageCrossPlatform( EXPAT REQUIRED )
message(STATUS "EXPAT_LIBRARIES = ${EXPAT_LIBRARIES}")

findPackageCrossPlatform( ZLIB REQUIRED )
message(STATUS "ZLIB_LIBRARIES = ${ZLIB_LIBRARIES}")

message(STATUS "CMAKE_DL_LIBS = ${CMAKE_DL_LIBS}")
