# Copyright (c) 2019 Denis Trofimov (den.a.trofimov@yandex.ru)
# Distributed under the MIT License.
# See accompanying file LICENSE.md or copy at http://opensource.org/licenses/MIT

## Search source files in folders
#addFolder( ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_NAME} "" ) # add main.cpp manually
#addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src ${PROJECT_NAME} "" ) # add main.cpp manually
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/storage ${PROJECT_NAME} "" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/config ${PROJECT_NAME} "" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/log ${PROJECT_NAME} "" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/net ${PROJECT_NAME} "" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/algo ${PROJECT_NAME} "" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR}/src/net/websockets ${PROJECT_NAME} "" )

set_vs_startup_project(${PROJECT_NAME}) # from Utils.cmake

set(CLANG_PATH CACHE STRING "/usr/lib/llvm-6.0/lib/clang/6.0.1")

set( THIRDPARTY_FILES
  ${Boost_INCLUDE_DIRS}
  CACHE INTERNAL "THIRDPARTY_FILES" )

set(THIRDPARTY_SOURCES
  "" # add here 3dparty deps
  CACHE INTERNAL "THIRDPARTY_SOURCES")

set(SOURCE_FILES ${THIRDPARTY_SOURCES} ${${PROJECT_NAME}_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp ${${PROJECT_NAME}_HEADERS})

# OTHER_IDE_FILES for IDE even if they have no build rules.
set(${PROJECT_NAME}_OTHER_IDE_FILES_EXTRA
  ".gitignore")
addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "cmake/*.cmake;cmake/*/*.*" )
#addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "assets/*.*;assets/*/*.*" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "client/*.*;client/*/*.*" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "docs/*.*;docs/*/*.*" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "scripts/*.sh;scripts/*/*.sh" )
addFolder( ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_NAME}_OTHER_IDE_FILES" "*.md;*.yml;*.json;*.cmake;*.in;*.txt;*.py" )
