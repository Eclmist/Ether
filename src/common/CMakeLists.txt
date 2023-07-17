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

set(ETHER_COMMON Common)

# =========================================================================== #
#                        ADD SOURCE FILES TO PROJECT                          #
# =========================================================================== #

file(GLOB_RECURSE common_files "*")
set(all_files ${common_files})

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${all_files})

# =========================================================================== #
#                          SET COMPILATION TARGETS                            #
# =========================================================================== #
add_library(${ETHER_COMMON} SHARED ${common_files})
target_compile_definitions(${ETHER_COMMON} PRIVATE "ETH_COMMON_LIB")



