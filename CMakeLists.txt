#
#    This file is part of Ether, an open-source DirectX12 renderer.
#   
#    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.
#   
#    Ether is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#   
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#   
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
#   

cmake_minimum_required (VERSION "3.18")

# =========================================================================== #
#                     PROJECT DEFINITIONS & PROPERTIES                        #
# =========================================================================== #

# Options
option(CONFIGURE_AS_TOOLMODE "Configure as Toolmode" OFF)
option(USE_PRECOMPILED_HEADERS "Use precompiled headers" ON)

if (CONFIGURE_AS_TOOLMODE)
    project(Ether.Toolmode)
else ()
    project(Ether)
endif()

# Sets the C++ versions
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Sets project output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win32")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win32")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/win32")

# Give WIN32 definition for all targets
add_compile_definitions("ETH_PLATFORM_WIN32")

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

include_directories("${CMAKE_SOURCE_DIR}/src")
include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/resource")

file(GLOB_RECURSE engine_files "${CMAKE_SOURCE_DIR}/src/engine/*")
file(GLOB_RECURSE api_files "${CMAKE_SOURCE_DIR}/src/api/*")
file(GLOB_RECURSE resources "${CMAKE_SOURCE_DIR}/resource/*")
file(GLOB root_files "${CMAKE_SOURCE_DIR}/src/*.h")

set(engine_files ${engine_files} ${root_files} ${api_files})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${engine_files})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

if (CONFIGURE_AS_TOOLMODE)
    set(ETHER_PROJECT "Ether.Toolmode")
else()
    set(ETHER_PROJECT "Ether")
endif()

add_library(${ETHER_PROJECT} SHARED ${engine_files})
target_compile_definitions(${ETHER_PROJECT} PRIVATE $<IF:$<BOOL:${CONFIGURE_AS_TOOLMODE}>, ETH_TOOLMODE, ETH_ENGINE>)

if(WIN32)
    # Enable multi-threaded compilation on Windows
    include(ProcessorCount)
    ProcessorCount(N)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")
endif()

# =========================================================================== #
#                            PRECOMPILED HEADERS                              #
# =========================================================================== #

if (USE_PRECOMPILED_HEADERS)
    target_precompile_headers(${ETHER_PROJECT} PRIVATE "${CMAKE_SOURCE_DIR}/src/pch.h")
endif()

# =========================================================================== #
#                              ADD SUBPROJECTS                                #
# =========================================================================== #

add_subdirectory(src/common)
add_subdirectory(src/graphics)

if (CONFIGURE_AS_TOOLMODE)
    add_subdirectory(src/toolmode)
else()
    add_subdirectory(sampleapp)
endif()

# =========================================================================== #
#                             LINK DEPENDENCIES                               #
# =========================================================================== #

target_link_libraries(${ETHER_PROJECT}
    Common
    Graphics
    "${CMAKE_SOURCE_DIR}/include/optick/OptickCore.lib"
)

