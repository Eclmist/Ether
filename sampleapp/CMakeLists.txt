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

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE sample_app src/*)
source_group(TREE ${CMAKE_CURRENT_LIST_DIR} FILES ${sample_app})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #

add_executable(SampleApp WIN32 ${sample_app})

# =========================================================================== #
#                                LINK LIBRARIES                               #
# =========================================================================== #

target_link_libraries(SampleApp ${ENGINE_PROJECT})
set_target_properties(SampleApp PROPERTIES LINK_FLAGS ${CMAKE_SOURCE_DIR}/resource/Ether.res)
