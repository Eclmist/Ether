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

# =========================================================================== #
#                     PROJECT DEFINITIONS & PROPERTIES                        #
# =========================================================================== #

set(ETHER_TOOLMODE Toolmode)

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/include")

file(GLOB_RECURSE toolmode_files "*")
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${toolmode_files})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #
add_executable(${ETHER_TOOLMODE} WIN32 ${toolmode_files})
target_compile_definitions(${ETHER_TOOLMODE} PRIVATE "ETH_TOOLMODE")

# =========================================================================== #
#                            PRECOMPILED HEADERS                              #
# =========================================================================== #

target_precompile_headers(${ETHER_TOOLMODE} PRIVATE "pch.h")

# =========================================================================== #
#                             LINK DEPENDENCIES                               #
# =========================================================================== #

target_link_libraries(${ETHER_TOOLMODE}
	ws2_32.lib
    ${ETHER_ENGINE}
    ${ETHER_COMMON}
)

