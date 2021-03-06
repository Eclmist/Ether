#
#    This file is part of Ether, an open-source DirectX12 renderer.
#   
#    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.
#   
#    Ether is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#   
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#   
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#   

cmake_minimum_required (VERSION "3.14.4")

# =========================================================================== #
#                     PROJECT DEFINITIONS & PROPERTIES                        #
# =========================================================================== #

# Setup the name of the solution. This will change some of the directories naming convention
# generated by CMake. Ignore CXX in the second param since CMake supports C++ by default
project (Ether)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# Ensure running on Windows
if (NOT WIN32)
    message(FATAL_ERROR "${PROJECT_NAME} is a DirectX 12 project and thus only supported on Windows 10! CMake will exit.")
endif ()

# Sets the C++ versions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS false)

# Set the project source directories. This is set when running the CMake GUI
set(ETH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Enable multi-thread compiling.
include(ProcessorCount)
ProcessorCount(N)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP${N}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

include_directories("${ETH_SOURCE_DIR}/src")
include_directories("${ETH_SOURCE_DIR}/include")

# Find all source files and group them by directory
# I have no clue how this actually works, this macro was taken from SORT
macro (source_group_by_dir source_files)
    set(sgbd_cur_dir ${ETH_SOURCE_DIR}/src)
    foreach (sgbd_file ${${source_files}})
	    string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
	    string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
	    string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
	    string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
	    if (sgbd_nogroup)
	        set(sgbd_group_name "\\" )
	    endif (sgbd_nogroup)
	    source_group(${sgbd_group_name} FILES ${sgbd_file})
    endforeach (sgbd_file)
endmacro (source_group_by_dir)

file(GLOB_RECURSE project_headers src/*.h)
file(GLOB_RECURSE project_cpps src/*.cpp)
file(GLOB_RECURSE project_shaders src/*.hlsl)
set_source_files_properties(${project_shaders} PROPERTIES VS_TOOL_OVERRIDE "None")

set(all_files ${project_headers} ${project_cpps} ${project_shaders})
source_group_by_dir(all_files)

# =========================================================================== #
#                   SETUP THIRD PARTY/EXTERN DEPENDENCIES                     #
# =========================================================================== #

include_directories("${ETH_SOURCE_DIR}/include")

file(GLOB_RECURSE imgui_headers include/imgui/*.h)
file(GLOB_RECURSE imgui_cpps include/imgui/*.cpp)
set(imgui_files ${imgui_headers} ${imgui_cpps})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

add_executable(${PROJECT_NAME} WIN32 ${all_files})
add_library(${PROJECT_NAME}_DLL SHARED ${all_files})

target_compile_definitions(${PROJECT_NAME} PUBLIC ETH_STANDALONE_BUILD)
target_compile_definitions(${PROJECT_NAME}_DLL PUBLIC ETH_EDITOR_BUILD)

set_target_properties (${PROJECT_NAME} PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${ETH_SOURCE_DIR}/bin/win32"
	RUNTIME_OUTPUT_NAME_RELEASE "${PROJECT_NAME}_win32_r"
	RUNTIME_OUTPUT_NAME_DEBUG "${PROJECT_NAME}_win32_d"
	RUNTIME_OUTPUT_NAME_RELWITHDEBINFO "${PROJECT_NAME}_win32_rx"
	RUNTIME_OUTPUT_NAME_MINSIZEREL "${PROJECT_NAME}_win32_rm"
)

set_target_properties (${PROJECT_NAME}_DLL PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${ETH_SOURCE_DIR}/bin/lib"
	RUNTIME_OUTPUT_NAME_RELEASE "${PROJECT_NAME}_r"
	RUNTIME_OUTPUT_NAME_DEBUG "${PROJECT_NAME}_d"
	RUNTIME_OUTPUT_NAME_RELWITHDEBINFO "${PROJECT_NAME}_rx"
	RUNTIME_OUTPUT_NAME_MINSIZEREL "${PROJECT_NAME}_rm"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${ETH_SOURCE_DIR}/src/graphic/shader
				   $<TARGET_FILE_DIR:${PROJECT_NAME}>/data/shader)

add_custom_command(TARGET ${PROJECT_NAME}_DLL POST_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${ETH_SOURCE_DIR}/src/graphic/shader
				   $<TARGET_FILE_DIR:${PROJECT_NAME}_DLL>/data/shader)


# =========================================================================== #
#                          LINK EXTERNAL LIBRARIES                            #
# =========================================================================== #

# Link DX12
target_link_libraries(${PROJECT_NAME} d3d12.lib dxgi.lib dxguid.lib)
target_link_libraries(${PROJECT_NAME}_DLL d3d12.lib dxgi.lib dxguid.lib)

# Link Imgui
ADD_LIBRARY(ImGui ${imgui_files})
target_link_libraries(${PROJECT_NAME} ImGui)
target_link_libraries(${PROJECT_NAME}_DLL ImGui)
