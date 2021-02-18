# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2006, Blender Foundation
# All rights reserved.
# ***** END GPL LICENSE BLOCK *****

# Otherwise we get warnings here that we cant fix in external projects
remove_strict_flags()

# Not a strict flag, but noisy for code we don't maintain
if(CMAKE_COMPILER_IS_GNUCC)
  remove_cc_flag(
    "-Wmisleading-indentation"
  )
endif()

if(NOT WITH_SYSTEM_GLEW)
  if(WITH_GLEW_ES)
    add_subdirectory(glew-es)
  else()
    add_subdirectory(glew)
  endif()
endif()

if(WITH_X11 AND WITH_GHOST_XDND)
  add_subdirectory(xdnd)
endif()

if(WITH_SDL AND WITH_SDL_DYNLOAD)
  add_subdirectory(sdlew)
endif()


