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

set(ETHER_GRAPHICS Graphics)
option(USE_PRECOMPILED_HEADERS "Use precompiled headers")

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

file(GLOB_RECURSE graphics_files "*")
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${graphics_files})

file(GLOB_RECURSE shader_files "${CMAKE_SOURCE_DIR}/*.hlsl")
set_source_files_properties(${shader_files} PROPERTIES VS_TOOL_OVERRIDE "None")

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #
add_library(${ETHER_GRAPHICS} SHARED ${graphics_files})

target_include_directories(${ETHER_GRAPHICS} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(${ETHER_GRAPHICS} SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_include_directories(${ETHER_GRAPHICS} SYSTEM PRIVATE "${CMAKE_SOURCE_DIR}/src/graphics/imgui") # to avoid needing to update imgui includes

target_compile_definitions(${ETHER_GRAPHICS} PRIVATE "ETH_GRAPHICS")
target_compile_definitions(${ETHER_GRAPHICS} PRIVATE "ETH_GRAPHICS_DX12")

# =========================================================================== #
#                            PRECOMPILED HEADERS                              #
# =========================================================================== #

if (USE_PRECOMPILED_HEADERS)
    target_precompile_headers(${ETHER_GRAPHICS} PRIVATE "${CMAKE_SOURCE_DIR}/src/graphics/pch.h")
endif()

# =========================================================================== #
#                                LINK LIBRARIES                               #
# =========================================================================== #

target_link_libraries(${ETHER_GRAPHICS} 
    Common
    d3d12.lib dxgi.lib dxguid.lib 
    "${CMAKE_SOURCE_DIR}/include/dxc/dxcompiler.lib"
    "${CMAKE_SOURCE_DIR}/include/pix/WinPixEventRuntime.lib"
    "${CMAKE_SOURCE_DIR}/include/optick/OptickCore.lib"

)


