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
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/src")

file(GLOB_RECURSE sample_app "${CMAKE_SOURCE_DIR}/sampleapp/src/*")

set(all_files ${sample_app})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

add_executable(SampleApp WIN32 ${sample_app})

# Set working directory to bin folder so Ether dlls can be found
set_property(TARGET SampleApp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

# =========================================================================== #
#                                LINK LIBRARIES                               #
# =========================================================================== #

target_link_libraries(SampleApp Engine)
set_target_properties(SampleApp PROPERTIES LINK_FLAGS "${CMAKE_SOURCE_DIR}/resource/Ether.res")
set_target_properties(SampleApp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

