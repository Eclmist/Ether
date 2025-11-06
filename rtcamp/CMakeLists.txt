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

file(GLOB_RECURSE rtcamp "${CMAKE_SOURCE_DIR}/rtcamp/*")

set(all_files ${rtcamp})
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${all_files})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

add_executable(RTCamp WIN32 ${rtcamp})

# Set working directory to bin folder so Ether dlls can be found
set_property(TARGET RTCamp PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

# =========================================================================== #
#                                LINK LIBRARIES                               #
# =========================================================================== #

target_link_libraries(RTCamp Engine)
set_target_properties(RTCamp PROPERTIES LINK_FLAGS "${CMAKE_SOURCE_DIR}/resource/Ether.res")
set_target_properties(RTCamp PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}")

# =========================================================================== #
#                              COPY REDIST BINS                               #
# =========================================================================== #

add_custom_command(TARGET RTCamp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/redist"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}"
    COMMENT "Copying contents of the redist folder to the working directory"
)


