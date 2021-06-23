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

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

file(GLOB_RECURSE sample_app src/*)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sample_app})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

add_executable(SampleApp WIN32 ${sample_app})
set_target_properties(SampleApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/bin/sampleapp>)
target_precompile_headers(SampleApp PRIVATE ${CMAKE_SOURCE_DIR}/src/system/pch.h)

add_custom_command(TARGET SampleApp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_SOURCE_DIR}/bin/enginelib/
                   $<TARGET_FILE_DIR:SampleApp>/)


# =========================================================================== #
#                                LINK LIBRARIES                               #
# =========================================================================== #

target_link_libraries(SampleApp Ether)

