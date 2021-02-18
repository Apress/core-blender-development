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
#
# ***** END GPL LICENSE BLOCK *****

#-----------------------------------------------------------------------------
# We don't allow in-source builds. This causes no end of troubles because
# all out-of-source builds will use the CMakeCache.txt file there and even
# build the libs and objects in it.

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  if(NOT DEFINED WITH_IN_SOURCE_BUILD)
    message(FATAL_ERROR
      "CMake generation for blender is not allowed within the source directory!"
      "\n Remove \"${CMAKE_SOURCE_DIR}/CMakeCache.txt\" and try again from another folder, e.g.:"
      "\n "
      "\n rm CMakeCache.txt"
      "\n cd .."
      "\n mkdir cmake-make"
      "\n cd cmake-make"
      "\n cmake ../blender"
      "\n "
      "\n Alternately define WITH_IN_SOURCE_BUILD to force this option (not recommended!)"
    )
  endif()
endif()

cmake_minimum_required(VERSION 3.5)

# Prever LEGACY OpenGL to eb compatible with all the existing releases and
# platforms which don't hare GLVND yet. Only do it if preference was not set
# externally.
if(NOT DEFINED OpenGL_GL_PREFERENCE)
  set(OpenGL_GL_PREFERENCE "LEGACY")
endif()

if(NOT EXECUTABLE_OUTPUT_PATH)
  set(FIRST_RUN TRUE)
else()
  set(FIRST_RUN FALSE)
endif()

# this starts out unset
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build_files/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build_files/cmake/platform")

# avoid having empty buildtype
if(NOT DEFINED CMAKE_BUILD_TYPE_INIT)
  set(CMAKE_BUILD_TYPE_INIT "Debug")
endif()

# Omit superfluous "Up-to-date" messages.
if(NOT DEFINED CMAKE_INSTALL_MESSAGE)
  set(CMAKE_INSTALL_MESSAGE "LAZY")
endif()

# quiet output for Makefiles, 'make -s' helps too
# set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)

# global compile definitions since add_definitions() adds for all.
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
  $<$<CONFIG:Debug>:DEBUG;_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  $<$<CONFIG:MinSizeRel>:NDEBUG>
  $<$<CONFIG:RelWithDebInfo>:NDEBUG>
)

#-----------------------------------------------------------------------------
# Set policy

# see "cmake --help-policy CMP0003"
# So library linking is more sane
cmake_policy(SET CMP0003 NEW)

# So BUILDINFO and BLENDERPATH strings are automatically quoted
cmake_policy(SET CMP0005 NEW)

# So syntax problems are errors
cmake_policy(SET CMP0010 NEW)

# Input directories must have CMakeLists.txt
cmake_policy(SET CMP0014 NEW)

# Silence draco warning on macOS, new policy works fine.
if(POLICY CMP0068)
  cmake_policy(SET CMP0068 NEW)
endif()

#-----------------------------------------------------------------------------
# Load some macros.
include(build_files/cmake/macros.cmake)


#-----------------------------------------------------------------------------
# Initialize project.

blender_project_hack_pre()

project(BlenderGhost)

blender_project_hack_post()

enable_testing()

#-----------------------------------------------------------------------------
# Redirect output files

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE INTERNAL "" FORCE)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE INTERNAL "" FORCE)
if(MSVC)
  set(TESTS_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/tests/$<CONFIG>/ CACHE INTERNAL "" FORCE)
else()
  set(TESTS_OUTPUT_DIR ${EXECUTABLE_OUTPUT_PATH}/tests/ CACHE INTERNAL "" FORCE)
endif()

#-----------------------------------------------------------------------------
# Set default config options

#get_blender_version()


#-----------------------------------------------------------------------------
# Options

# First platform specific non-cached vars
if(UNIX AND NOT (APPLE OR HAIKU))
  set(WITH_X11 ON)
endif()

# Blender internal features
if(${CMAKE_VERSION} VERSION_LESS 2.8.8)
  # add_library OBJECT arg unsupported
  set(WITH_BUILDINFO OFF)
endif()
set(BUILDINFO_OVERRIDE_DATE "" CACHE STRING "Use instead of the current date for reproducible builds (empty string disables this option)")
set(BUILDINFO_OVERRIDE_TIME "" CACHE STRING "Use instead of the current time for reproducible builds (empty string disables this option)")
set(CPACK_OVERRIDE_PACKAGENAME "" CACHE STRING "Use instead of the standard packagename (empty string disables this option)")


# GHOST Windowing Library Options
option(WITH_GHOST_DEBUG   "Enable debugging output for the GHOST library" OFF)
mark_as_advanced(WITH_GHOST_DEBUG)

option(WITH_GHOST_SDL    "Enable building Blender against SDL for windowing rather than the native APIs" OFF)
mark_as_advanced(WITH_GHOST_SDL)

if(WITH_X11)
  option(WITH_GHOST_XDND    "Enable drag'n'drop support on X11 using XDND protocol" ON)
endif()

# Misc...
if(WITH_X11)
  option(WITH_X11_XINPUT    "Enable X11 Xinput (tablet support and unicode input)"  ON)
  option(WITH_X11_XF86VMODE "Enable X11 video mode switching"                       ON)
  option(WITH_X11_XFIXES    "Enable X11 XWayland cursor warping workaround"         ON)
  option(WITH_X11_ALPHA     "Enable X11 transparent background"                     ON)
endif()

if(UNIX AND NOT APPLE)
  option(WITH_SYSTEM_GLEW "Use GLEW OpenGL wrapper library provided by the operating system" OFF)
  option(WITH_SYSTEM_GLES "Use OpenGL ES library provided by the operating system"           ON)
else()
  # not an option for other OS's
  set(WITH_SYSTEM_GLEW OFF)
  set(WITH_SYSTEM_GLES OFF)
endif()


#if(UNIX AND NOT APPLE)
#  option(WITH_SYSTEM_EIGEN3 "Use the systems Eigen3 library" OFF)
#endif()

# disable for now, but plan to support on all platforms eventually
option(WITH_MEM_JEMALLOC   "Enable malloc replacement (http://www.canonware.com/jemalloc)" ON)
mark_as_advanced(WITH_MEM_JEMALLOC)

# OpenGL

option(WITH_OPENGL              "When off limits visibility of the opengl headers to just bf_gpu and gawain (temporary option for development purposes)" ON)
option(WITH_GLEW_ES             "Switches to experimental copy of GLEW that has support for OpenGL ES. (temporary option for development purposes)" OFF)
option(WITH_GL_EGL              "Use the EGL OpenGL system library instead of the platform specific OpenGL system library (CGL, glX, or WGL)"       OFF)
option(WITH_GL_PROFILE_ES20     "Support using OpenGL ES 2.0. (through either EGL or the AGL/WGL/XGL 'es20' profile)"                               OFF)

mark_as_advanced(
  WITH_OPENGL
  WITH_GLEW_ES
  WITH_GL_EGL
  WITH_GL_PROFILE_ES20
)

if(WIN32)
  option(WITH_GL_ANGLE "Link with the ANGLE library, an OpenGL ES 2.0 implementation based on Direct3D, instead of the system OpenGL library." OFF)
  mark_as_advanced(WITH_GL_ANGLE)
endif()

if(WITH_GLEW_ES AND WITH_SYSTEM_GLEW)
  message(WARNING Ignoring WITH_SYSTEM_GLEW and using WITH_GLEW_ES)
  set(WITH_SYSTEM_GLEW OFF)
endif()

if(WIN32)
  getDefaultWindowsPrefixBase(CMAKE_GENERIC_PROGRAM_FILES)
  set(CPACK_INSTALL_PREFIX ${CMAKE_GENERIC_PROGRAM_FILES}/${})
endif()

# Compiler toolchain
if(CMAKE_COMPILER_IS_GNUCC)
  option(WITH_LINKER_GOLD "Use ld.gold linker which is usually faster than ld.bfd" ON)
  mark_as_advanced(WITH_LINKER_GOLD)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
  option(WITH_COMPILER_ASAN "Build and link against address sanitizer (only for Debug & RelWithDebInfo targets)." OFF)
  mark_as_advanced(WITH_COMPILER_ASAN)

  if(WITH_COMPILER_ASAN)
    set(_asan_defaults "\
-fsanitize=address \
-fsanitize=bool \
-fsanitize=bounds \
-fsanitize=enum \
-fsanitize=float-cast-overflow \
-fsanitize=float-divide-by-zero \
-fsanitize=nonnull-attribute \
-fsanitize=returns-nonnull-attribute \
-fsanitize=signed-integer-overflow \
-fsanitize=undefined \
-fsanitize=vla-bound \
-fno-sanitize=alignment \
")

    if(NOT MSVC) # not all sanitizers are supported with clang-cl, these two however are very vocal about it
      set(_asan_defaults "${_asan_defaults} -fsanitize=leak -fsanitize=object-size" )
    endif()
    set(COMPILER_ASAN_CFLAGS "${_asan_defaults}" CACHE STRING "C flags for address sanitizer")
    mark_as_advanced(COMPILER_ASAN_CFLAGS)
    set(COMPILER_ASAN_CXXFLAGS "${_asan_defaults}" CACHE STRING "C++ flags for address sanitizer")
    mark_as_advanced(COMPILER_ASAN_CXXFLAGS)

    unset(_asan_defaults)

    if(NOT MSVC)
      find_library(COMPILER_ASAN_LIBRARY asan ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})
    else()
      find_library(
        COMPILER_ASAN_LIBRARY NAMES clang_rt.asan-x86_64
        PATHS
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\LLVM\\LLVM;]/lib/clang/7.0.0/lib/windows
        [HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\LLVM\\LLVM;]/lib/clang/6.0.0/lib/windows
      )
    endif()
    mark_as_advanced(COMPILER_ASAN_LIBRARY)
  endif()
endif()

if(WIN32)
  # Use hardcoded paths or find_package to find externals
  option(WITH_WINDOWS_FIND_MODULES "Use find_package to locate libraries" OFF)
  mark_as_advanced(WITH_WINDOWS_FIND_MODULES)

  option(WINDOWS_USE_VISUAL_STUDIO_PROJECT_FOLDERS "Organize the visual studio projects according to source folder structure." ON)
  mark_as_advanced(WINDOWS_USE_VISUAL_STUDIO_PROJECT_FOLDERS)

  option(WINDOWS_USE_VISUAL_STUDIO_SOURCE_FOLDERS "Organize the source files in filters matching the source folders." ON)
  mark_as_advanced(WINDOWS_USE_VISUAL_STUDIO_SOURCE_FOLDERS)

  option(WINDOWS_PYTHON_DEBUG "Include the files needed for debugging python scripts with visual studio 2017+." OFF)
  mark_as_advanced(WINDOWS_PYTHON_DEBUG)

  option(WITH_WINDOWS_BUNDLE_CRT "Bundle the C runtime for install free distribution." ON)
  mark_as_advanced(WITH_WINDOWS_BUNDLE_CRT)

endif()

# The following only works with the Ninja generator in CMake >= 3.0.
if("${CMAKE_GENERATOR}" MATCHES "Ninja")
  option(WITH_NINJA_POOL_JOBS
         "Enable Ninja pools of jobs, to try to ease building on machines with 16GB of RAM or less (if not yet defined, will try to set best values based on detected machine specifications)."
         OFF)
  mark_as_advanced(WITH_NINJA_POOL_JOBS)
endif()

if(UNIX AND NOT APPLE)
  option(WITH_CXX11_ABI "Use native C++11 ABI of compiler" ON)
  mark_as_advanced(WITH_CXX11_ABI)
endif()

# Installation process.
option(POSTINSTALL_SCRIPT "Run given CMake script after installation process" OFF)
mark_as_advanced(POSTINSTALL_SCRIPT)

# end option(...)



# By default we want to install to the directory we are compiling our executables
# unless specified otherwise, which we currently do not allow
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  if(WIN32)
    set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE} CACHE PATH "default install path" FORCE)
  elseif(APPLE)
    set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH}/\${BUILD_TYPE} CACHE PATH "default install path" FORCE)
  else()
    if(WITH_INSTALL_PORTABLE)
      set(CMAKE_INSTALL_PREFIX ${EXECUTABLE_OUTPUT_PATH} CACHE PATH "default install path" FORCE)
    endif()
  endif()
endif()



# Apple

if(APPLE)
  include(platform_apple_xcode)
endif()


#-----------------------------------------------------------------------------
# Check for conflicting/unsupported configurations
if(NOT WITH_SDL AND WITH_GHOST_SDL)
  message(FATAL_ERROR "WITH_GHOST_SDL requires WITH_SDL")
endif()

if(WITH_GHOST_SDL OR WITH_HEADLESS)
  set(WITH_X11           OFF)
  set(WITH_X11_XINPUT    OFF)
  set(WITH_X11_XF86VMODE OFF)
  set(WITH_X11_XFIXES    OFF)
  set(WITH_X11_ALPHA     OFF)
  set(WITH_GHOST_XDND    OFF)
  set(WITH_INPUT_IME     OFF)
endif()

if(WITH_CPU_SSE)
  TEST_SSE_SUPPORT(COMPILER_SSE_FLAG COMPILER_SSE2_FLAG)
else()
  message(STATUS "SSE and SSE2 optimizations are DISABLED!")
  set(COMPILER_SSE_FLAG)
  set(COMPILER_SSE2_FLAG)
endif()

#-----------------------------------------------------------------------------
# Initialize un-cached vars, avoid unused warning

# linux only, not cached
set(WITH_BINRELOC OFF)

# MACOSX only, set to avoid uninitialized
set(EXETYPE "")

# C/C++ flags
set(PLATFORM_CFLAGS)

# these are added to later on.
set(C_WARNINGS)
set(CXX_WARNINGS)

# for gcc -Wno-blah-blah
set(C_REMOVE_STRICT_FLAGS)
set(CXX_REMOVE_STRICT_FLAGS)

# libraries to link the binary with passed to target_link_libraries()
# known as LLIBS to scons
set(PLATFORM_LINKLIBS "")

# Added to linker flags in setup_liblinks
# - CMAKE_EXE_LINKER_FLAGS
# - CMAKE_EXE_LINKER_FLAGS_DEBUG
set(PLATFORM_LINKFLAGS "")
set(PLATFORM_LINKFLAGS_DEBUG "")

if(NOT CMAKE_BUILD_TYPE MATCHES "Release")
  if(WITH_COMPILER_ASAN)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${COMPILER_ASAN_CFLAGS}")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${COMPILER_ASAN_CFLAGS}")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${COMPILER_ASAN_CXXFLAGS}")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMPILER_ASAN_CXXFLAGS}")
    if(MSVC)
      set(COMPILER_ASAN_LINKER_FLAGS "/FUNCTIONPADMIN:6")
    endif()
    if(COMPILER_ASAN_LIBRARY)
      set(PLATFORM_LINKLIBS "${PLATFORM_LINKLIBS};${COMPILER_ASAN_LIBRARY}")
      set(PLATFORM_LINKFLAGS "${COMPILER_ASAN_LIBRARY} ${COMPILER_ASAN_LINKER_FLAGS}")
      set(PLATFORM_LINKFLAGS_DEBUG "${COMPILER_ASAN_LIBRARY} ${COMPILER_ASAN_LINKER_FLAGS}")
    endif()
  endif()
endif()

# ----------------------------------------------------------------------------
# Main Platform Checks
#
# - UNIX
# - WIN32
# - APPLE

if(UNIX AND NOT APPLE)
  include(platform_unix)
elseif(WIN32)
  include(platform_win32)
elseif(APPLE)
  include(platform_apple)
endif()

# set the endian define
if(MSVC)
  # for some reason this fails on msvc
  add_definitions(-D__LITTLE_ENDIAN__)

  # OSX-Note: as we do cross-compiling with specific set architecture,
  # endianess-detection and auto-setting is counterproductive
  # so we just set endianness according CMAKE_OSX_ARCHITECTURES

elseif(CMAKE_OSX_ARCHITECTURES MATCHES i386 OR CMAKE_OSX_ARCHITECTURES MATCHES x86_64)
  add_definitions(-D__LITTLE_ENDIAN__)
elseif(CMAKE_OSX_ARCHITECTURES MATCHES ppc OR CMAKE_OSX_ARCHITECTURES MATCHES ppc64)
  add_definitions(-D__BIG_ENDIAN__)

else()
  include(TestBigEndian)
  test_big_endian(_SYSTEM_BIG_ENDIAN)
  if(_SYSTEM_BIG_ENDIAN)
    add_definitions(-D__BIG_ENDIAN__)
  else()
    add_definitions(-D__LITTLE_ENDIAN__)
  endif()
  unset(_SYSTEM_BIG_ENDIAN)
endif()


#-----------------------------------------------------------------------------
# Configure OpenGL.

find_package(OpenGL)
blender_include_dirs_sys("${OPENGL_INCLUDE_DIR}")

if(WITH_OPENGL)
  add_definitions(-DWITH_OPENGL)
endif()

if(WITH_SYSTEM_GLES)
  find_package_wrapper(OpenGLES)
endif()

if(WITH_GL_PROFILE_ES20)
  if(WITH_SYSTEM_GLES)
    if(NOT OPENGLES_LIBRARY)
      message(FATAL_ERROR
        "Unable to find OpenGL ES libraries. "
        "Install them or disable WITH_SYSTEM_GLES."
      )
    endif()

    list(APPEND BLENDER_GL_LIBRARIES "${OPENGLES_LIBRARY}")

  else()
    set(OPENGLES_LIBRARY "" CACHE FILEPATH "OpenGL ES 2.0 library file")
    mark_as_advanced(OPENGLES_LIBRARY)

    list(APPEND BLENDER_GL_LIBRARIES "${OPENGLES_LIBRARY}")

    if(NOT OPENGLES_LIBRARY)
      message(FATAL_ERROR
        "To compile WITH_GL_EGL you need to set OPENGLES_LIBRARY "
        "to the file path of an OpenGL ES 2.0 library."
      )
    endif()

  endif()

  if(WIN32)
    # Setup paths to files needed to install and redistribute Windows Blender with OpenGL ES

    set(OPENGLES_DLL "" CACHE FILEPATH "OpenGL ES 2.0 redistributable DLL file")
    mark_as_advanced(OPENGLES_DLL)

    if(NOT OPENGLES_DLL)
      message(FATAL_ERROR
        "To compile WITH_GL_PROFILE_ES20 you need to set OPENGLES_DLL to the file "
        "path of an OpenGL ES 2.0 runtime dynamic link library (DLL)."
      )
    endif()

    if(WITH_GL_ANGLE)
      list(APPEND GL_DEFINITIONS -DWITH_ANGLE)

      set(D3DCOMPILER_DLL "" CACHE FILEPATH "Direct3D Compiler redistributable DLL file (needed by ANGLE)")

      get_filename_component(D3DCOMPILER_FILENAME "${D3DCOMPILER_DLL}" NAME)
      list(APPEND GL_DEFINITIONS "-DD3DCOMPILER=\"\\\"${D3DCOMPILER_FILENAME}\\\"\"")

      mark_as_advanced(D3DCOMPILER_DLL)

      if(D3DCOMPILER_DLL STREQUAL "")
        message(FATAL_ERROR
          "To compile WITH_GL_ANGLE you need to set D3DCOMPILER_DLL to the file "
          "path of a copy of the DirectX redistributable DLL file: D3DCompiler_46.dll"
        )
      endif()

    endif()

  endif()

else()
  if(OpenGL_GL_PREFERENCE STREQUAL "LEGACY" AND OPENGL_gl_LIBRARY)
    list(APPEND BLENDER_GL_LIBRARIES ${OPENGL_gl_LIBRARY})
  else()
    list(APPEND BLENDER_GL_LIBRARIES ${OPENGL_opengl_LIBRARY} ${OPENGL_glx_LIBRARY})
  endif()
endif()

if(WITH_GL_EGL)
  find_package(OpenGL REQUIRED EGL)
  list(APPEND BLENDER_GL_LIBRARIES OpenGL::EGL)

  list(APPEND GL_DEFINITIONS -DWITH_GL_EGL -DGLEW_EGL -DGLEW_INC_EGL)

  if(WITH_SYSTEM_GLES)
    if(NOT OPENGLES_EGL_LIBRARY)
      message(FATAL_ERROR
        "Unable to find OpenGL ES libraries. "
        "Install them or disable WITH_SYSTEM_GLES."
      )
    endif()

    list(APPEND BLENDER_GL_LIBRARIES ${OPENGLES_EGL_LIBRARY})

  else()
    set(OPENGLES_EGL_LIBRARY "" CACHE FILEPATH "EGL library file")
    mark_as_advanced(OPENGLES_EGL_LIBRARY)

    list(APPEND BLENDER_GL_LIBRARIES "${OPENGLES_LIBRARY}" "${OPENGLES_EGL_LIBRARY}")

    if(NOT OPENGLES_EGL_LIBRARY)
      message(FATAL_ERROR
        "To compile WITH_GL_EGL you need to set OPENGLES_EGL_LIBRARY "
        "to the file path of an EGL library."
      )
    endif()

  endif()

  if(WIN32)
    # Setup paths to files needed to install and redistribute Windows Blender with OpenGL ES

    set(OPENGLES_EGL_DLL "" CACHE FILEPATH "EGL redistributable DLL file")
    mark_as_advanced(OPENGLES_EGL_DLL)

    if(NOT OPENGLES_EGL_DLL)
      message(FATAL_ERROR
        "To compile WITH_GL_EGL you need to set OPENGLES_EGL_DLL "
        "to the file path of an EGL runtime dynamic link library (DLL)."
      )
    endif()

  endif()

endif()

if(WITH_GL_PROFILE_ES20)
  list(APPEND GL_DEFINITIONS -DWITH_GL_PROFILE_ES20)
else()
  list(APPEND GL_DEFINITIONS -DWITH_GL_PROFILE_CORE)
endif()

#-----------------------------------------------------------------------------
# Configure GLEW

if(WITH_SYSTEM_GLEW)
  find_package(GLEW)

  # Note: There is an assumption here that the system GLEW is not a static library.

  if(NOT GLEW_FOUND)
    message(FATAL_ERROR "GLEW is required to build Blender. Install it or disable WITH_SYSTEM_GLEW.")
  endif()

  set(GLEW_INCLUDE_PATH "${GLEW_INCLUDE_DIR}")
  set(BLENDER_GLEW_LIBRARIES ${GLEW_LIBRARY})
else()
  if(WITH_GLEW_ES)
    set(GLEW_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/extern/glew-es/include")

    list(APPEND GL_DEFINITIONS -DGLEW_STATIC -DWITH_GLEW_ES)

    # These definitions remove APIs from glew.h, making GLEW smaller, and catching unguarded API usage
    if(WITH_GL_PROFILE_ES20)
      list(APPEND GL_DEFINITIONS -DGLEW_ES_ONLY)
    else()
      # No ES functions are needed
      list(APPEND GL_DEFINITIONS -DGLEW_NO_ES)
    endif()

    if(WITH_GL_PROFILE_ES20)
      if(WITH_GL_EGL)
        list(APPEND GL_DEFINITIONS -DGLEW_USE_LIB_ES20)
      endif()

      # ToDo: This is an experiment to eliminate ES 1 symbols,
      # GLEW doesn't really properly provide this level of control
      # (for example, without modification it eliminates too many symbols)
      # so there are lots of modifications to GLEW to make this work,
      # and no attempt to make it work beyond Blender at this point.
      list(APPEND GL_DEFINITIONS -DGL_ES_VERSION_1_0=0 -DGL_ES_VERSION_CL_1_1=0 -DGL_ES_VERSION_CM_1_1=0)
    endif()

    set(BLENDER_GLEW_LIBRARIES extern_glew_es bf_intern_glew_mx)

  else()
    set(GLEW_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/extern/glew/include")

    list(APPEND GL_DEFINITIONS -DGLEW_STATIC)

    # This won't affect the non-experimental glew library,
    # but is used for conditional compilation elsewhere.
    list(APPEND GL_DEFINITIONS -DGLEW_NO_ES)

    set(BLENDER_GLEW_LIBRARIES extern_glew)

  endif()

endif()

list(APPEND GL_DEFINITIONS -DGLEW_NO_GLU)

#-----------------------------------------------------------------------------
# Extra compile flags

if(CMAKE_COMPILER_IS_GNUCC)

  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ALL -Wall)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_CAST_ALIGN -Wcast-align)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ERROR_IMPLICIT_FUNCTION_DECLARATION -Werror=implicit-function-declaration)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ERROR_VLA -Werror=vla)
  # system headers sometimes do this, disable for now, was: -Werror=strict-prototypes
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_STRICT_PROTOTYPES  -Wstrict-prototypes)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_MISSING_PROTOTYPES -Wmissing-prototypes)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_POINTER_ARITH -Wpointer-arith)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_UNUSED_PARAMETER -Wunused-parameter)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_WRITE_STRINGS -Wwrite-strings)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_LOGICAL_OP -Wlogical-op)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_UNDEF -Wundef)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_INIT_SELF -Winit-self)  # needs -Wuninitialized
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_MISSING_INCLUDE_DIRS -Wmissing-include-dirs)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_DIV_BY_ZERO -Wno-div-by-zero)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_TYPE_LIMITS -Wtype-limits)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_FORMAT_SIGN -Wformat-signedness)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_RESTRICT -Wrestrict)

  # C-only.
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_NULL -Wnonnull)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ABSOLUTE_VALUE -Wabsolute-value)

  # gcc 4.2 gives annoying warnings on every file with this
  if(NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "4.3")
    ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_UNINITIALIZED -Wuninitialized)
  endif()

  # versions before gcc4.6 give many BLI_math warnings
  if(NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "4.6")
    ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_REDUNDANT_DECLS       -Wredundant-decls)
    ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_REDUNDANT_DECLS -Wredundant-decls)
  endif()

  # versions before gcc4.8 include global name-space.
  if(NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "4.8")
    ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_SHADOW -Wshadow)
  endif()

  # disable because it gives warnings for printf() & friends.
  # ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_DOUBLE_PROMOTION -Wdouble-promotion -Wno-error=double-promotion)

  if(NOT APPLE)
    ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_ERROR_UNUSED_BUT_SET_VARIABLE -Wno-error=unused-but-set-variable)
  endif()

  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_ALL -Wall)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_INVALID_OFFSETOF -Wno-invalid-offsetof)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_SIGN_COMPARE -Wno-sign-compare)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_LOGICAL_OP -Wlogical-op)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_INIT_SELF -Winit-self)  # needs -Wuninitialized
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_MISSING_INCLUDE_DIRS -Wmissing-include-dirs)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_DIV_BY_ZERO -Wno-div-by-zero)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_TYPE_LIMITS -Wtype-limits)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_POINTER_ARITH -Wpointer-arith)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_UNUSED_PARAMETER -Wunused-parameter)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_WRITE_STRINGS -Wwrite-strings)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_UNDEF -Wundef)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_FORMAT_SIGN -Wformat-signedness)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_RESTRICT -Wrestrict)

  # gcc 4.2 gives annoying warnings on every file with this
  if(NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "4.3")
    ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_UNINITIALIZED -Wuninitialized)
  endif()

  # causes too many warnings
  if(NOT APPLE)
    ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_UNDEF -Wundef)
    ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_MISSING_DECLARATIONS -Wmissing-declarations)
  endif()

  # Use 'ATTR_FALLTHROUGH' macro to suppress.
  if(CMAKE_COMPILER_IS_GNUCC AND (NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "7.0"))
    ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_IMPLICIT_FALLTHROUGH -Wimplicit-fallthrough=5)
    ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_IMPLICIT_FALLTHROUGH -Wimplicit-fallthrough=5)
  endif()

  # flags to undo strict flags
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_DEPRECATED_DECLARATIONS -Wno-deprecated-declarations)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_PARAMETER        -Wno-unused-parameter)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_FUNCTION         -Wno-unused-function)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_TYPE_LIMITS             -Wno-type-limits)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_INT_IN_BOOL_CONTEXT     -Wno-int-in-bool-context)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_FORMAT                  -Wno-format)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_SWITCH                  -Wno-switch)

  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_CLASS_MEMACCESS     -Wno-class-memaccess)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_COMMENT             -Wno-comment)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_TYPEDEFS     -Wno-unused-local-typedefs)

  if(CMAKE_COMPILER_IS_GNUCC AND (NOT "${CMAKE_C_COMPILER_VERSION}" VERSION_LESS "7.0"))
    ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_IMPLICIT_FALLTHROUGH    -Wno-implicit-fallthrough)
  endif()

  if(NOT APPLE)
    ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_ERROR_UNUSED_BUT_SET_VARIABLE -Wno-error=unused-but-set-variable)
  endif()

elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")

  # strange, clang complains these are not supported, but then uses them.
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ALL -Wall)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ERROR_IMPLICIT_FUNCTION_DECLARATION -Werror=implicit-function-declaration)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ERROR_RETURN_TYPE  -Werror=return-type)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_AUTOLOGICAL_COMPARE -Wno-tautological-compare)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_CHAR_SUBSCRIPTS -Wno-char-subscripts)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_STRICT_PROTOTYPES  -Wstrict-prototypes)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_MISSING_PROTOTYPES -Wmissing-prototypes)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_UNUSED_PARAMETER -Wunused-parameter)

  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_ALL -Wall)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_AUTOLOGICAL_COMPARE -Wno-tautological-compare)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_UNKNOWN_PRAGMAS     -Wno-unknown-pragmas)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_CHAR_SUBSCRIPTS     -Wno-char-subscripts)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_OVERLOADED_VIRTUAL  -Wno-overloaded-virtual)  # we get a lot of these, if its a problem a dev needs to look into it.
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_SIGN_COMPARE        -Wno-sign-compare)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_INVALID_OFFSETOF    -Wno-invalid-offsetof)

  # gives too many unfixable warnings
  # ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_UNUSED_MACROS      -Wunused-macros)
  # ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_UNUSED_MACROS          -Wunused-macros)

  # flags to undo strict flags
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_PARAMETER -Wno-unused-parameter)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_VARIABLE  -Wno-unused-variable)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_MACROS    -Wno-unused-macros)

  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_VARIABLE_DECLARATIONS -Wno-missing-variable-declarations)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_INCOMPAT_PTR_DISCARD_QUAL -Wno-incompatible-pointer-types-discards-qualifiers)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNUSED_FUNCTION -Wno-unused-function)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_INT_TO_VOID_POINTER_CAST -Wno-int-to-void-pointer-cast)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_PROTOTYPES -Wno-missing-prototypes)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_DUPLICATE_ENUM -Wno-duplicate-enum)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_UNDEF -Wno-undef)
  ADD_CHECK_C_COMPILER_FLAG(C_REMOVE_STRICT_FLAGS C_WARN_NO_MISSING_NORETURN -Wno-missing-noreturn)

  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_PRIVATE_FIELD -Wno-unused-private-field)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_CXX11_NARROWING -Wno-c++11-narrowing)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_NON_VIRTUAL_DTOR -Wno-non-virtual-dtor)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_MACROS -Wno-unused-macros)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_REORDER -Wno-reorder)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_COMMENT -Wno-comment)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNUSED_TYPEDEFS -Wno-unused-local-typedefs)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_UNDEFINED_VAR_TEMPLATE -Wno-undefined-var-template)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_REMOVE_STRICT_FLAGS CXX_WARN_NO_INSTANTIATION_AFTER_SPECIALIZATION -Wno-instantiation-after-specialization)

elseif(CMAKE_C_COMPILER_ID MATCHES "Intel")

  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_ALL -Wall)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_POINTER_ARITH -Wpointer-arith)
  ADD_CHECK_C_COMPILER_FLAG(C_WARNINGS C_WARN_NO_UNKNOWN_PRAGMAS -Wno-unknown-pragmas)

  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_ALL -Wall)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_INVALID_OFFSETOF -Wno-invalid-offsetof)
  ADD_CHECK_CXX_COMPILER_FLAG(CXX_WARNINGS CXX_WARN_NO_SIGN_COMPARE -Wno-sign-compare)

  # disable numbered, false positives
  set(C_WARNINGS "${C_WARNINGS} -wd188,186,144,913,556,858,597,177,1292,167,279,592,94,2722,3199")
  set(CXX_WARNINGS "${CXX_WARNINGS} -wd188,186,144,913,556,858,597,177,1292,167,279,592,94,2722,3199")
elseif(CMAKE_C_COMPILER_ID MATCHES "MSVC")
  # most msvc warnings are C & C++
  set(_WARNINGS
    # warning level:
    "/W3"
    "/w34062"  # switch statement contains 'default' but no 'case' labels
    "/w34115"  # 'type' : named type definition in parentheses
    "/w34189"  # local variable is initialized but not referenced
    # disable:
    "/wd4018"  # signed/unsigned mismatch
    "/wd4146"  # unary minus operator applied to unsigned type, result still unsigned
    "/wd4065"  # switch statement contains 'default' but no 'case' labels
    "/wd4127"  # conditional expression is constant
    "/wd4181"  # qualifier applied to reference type; ignored
    "/wd4200"  # zero-sized array in struct/union
    "/wd4244"  # conversion from 'type1' to 'type2', possible loss of data
    "/wd4267"  # conversion from 'size_t' to 'type', possible loss of data
    "/wd4305"  # truncation from 'type1' to 'type2'
    "/wd4800"  # forcing value to bool 'true' or 'false'
    "/wd4828"  # The file contains a character that is illegal
    "/wd4996"  # identifier was declared deprecated
    "/wd4661"  # no suitable definition provided for explicit template instantiation request
    # errors:
    "/we4013"  # 'function' undefined; assuming extern returning int
    "/we4133"  # incompatible pointer types
    "/we4431"  # missing type specifier - int assumed
  )

  if(MSVC_VERSION GREATER_EQUAL 1911)
    # see https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/c5038?view=vs-2017
    set(_WARNINGS "${_WARNINGS} /w35038") # order of initialization in c++ constructors
  endif()

  string(REPLACE ";" " " _WARNINGS "${_WARNINGS}")
  set(C_WARNINGS "${_WARNINGS}")
  set(CXX_WARNINGS "${_WARNINGS}")
  unset(_WARNINGS)
endif()

if(MSVC)
  # MSVC needs to be tested first, since clang on windows will
  # match the compiler test below but clang-cl does not accept -std=c++11
  # since it is on by default and cannot be turned off.
  #
  # Nothing special is needed, C++11 features are available by default.
elseif(
  CMAKE_COMPILER_IS_GNUCC OR
  CMAKE_C_COMPILER_ID MATCHES "Clang" OR
  CMAKE_C_COMPILER_ID MATCHES "Intel"
)
  # TODO(sergey): Do we want c++11 or gnu-c++11 here?
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(FATAL_ERROR "Unknown compiler ${CMAKE_C_COMPILER_ID}, can't enable C++11 build")
endif()

# Visual Studio has all standards it supports available by default
# Clang on windows copies this behavior and does not support these switches
if(
  CMAKE_COMPILER_IS_GNUCC OR
  (CMAKE_C_COMPILER_ID MATCHES "Clang" AND (NOT MSVC)) OR
  (CMAKE_C_COMPILER_ID MATCHES "Intel")
)
  # Use C11 + GNU extensions, works with GCC, Clang, ICC
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif()

if(UNIX AND NOT APPLE)
  if(NOT WITH_CXX11_ABI)
    set(PLATFORM_CFLAGS "${PLATFORM_CFLAGS} -D_GLIBCXX_USE_CXX11_ABI=0")
  endif()
endif()

# Include warnings first, so its possible to disable them with user defined flags
# eg: -Wno-uninitialized
set(CMAKE_C_FLAGS "${C_WARNINGS} ${CMAKE_C_FLAGS} ${PLATFORM_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CXX_WARNINGS} ${CMAKE_CXX_FLAGS} ${PLATFORM_CFLAGS}")

#-------------------------------------------------------------------------------
# Global Defines

# better not set includes here but this debugging option is off by default.
if(WITH_CXX_GUARDEDALLOC)
  include_directories(${CMAKE_SOURCE_DIR}/intern/guardedalloc)
  add_definitions(-DWITH_CXX_GUARDEDALLOC)
endif()

if(WITH_ASSERT_ABORT)
  add_definitions(-DWITH_ASSERT_ABORT)
endif()

# message(STATUS "Using CFLAGS: ${CMAKE_C_FLAGS}")
# message(STATUS "Using CXXFLAGS: ${CMAKE_CXX_FLAGS}")

#-----------------------------------------------------------------------------
# Libraries
#cmaadd_subdirectory(extern)
add_subdirectory(intern)

#-----------------------------------------------------------------------------
# Print Final Configuration

if(FIRST_RUN)

  set(_config_msg "\nBlender Configuration\n=====================")

  function(info_cfg_option
    _setting
    )

    set(_msg "  - ${_setting}")
    string(LENGTH "${_msg}" _len)
    while("32" GREATER "${_len}")
      set(_msg "${_msg} ")
      math(EXPR _len "${_len} + 1")
    endwhile()

    set(_config_msg "${_config_msg}\n${_msg}${${_setting}}" PARENT_SCOPE)
  endfunction()

  function(info_cfg_text
    _text
    )

    set(_config_msg "${_config_msg}\n\n  ${_text}" PARENT_SCOPE)
  endfunction()

  message(STATUS "C Compiler:   \"${CMAKE_C_COMPILER_ID}\"")
  message(STATUS "C++ Compiler: \"${CMAKE_CXX_COMPILER_ID}\"")
  
  info_cfg_text("System Options:")
  info_cfg_option(WITH_INSTALL_PORTABLE)
  info_cfg_option(WITH_X11_ALPHA)
  info_cfg_option(WITH_X11_XF86VMODE)
  info_cfg_option(WITH_X11_XFIXES)
  info_cfg_option(WITH_X11_XINPUT)
  info_cfg_option(WITH_MEM_JEMALLOC)
  info_cfg_option(WITH_MEM_VALGRIND)
  info_cfg_option(WITH_SYSTEM_GLEW)
  
  info_cfg_text("OpenGL:")
  info_cfg_option(WITH_GLEW_ES)
  info_cfg_option(WITH_GL_EGL)
  info_cfg_option(WITH_GL_PROFILE_ES20)
  if(WIN32)
    info_cfg_option(WITH_GL_ANGLE)
  endif()
  
  info_cfg_text("Audio:")
  info_cfg_option(WITH_SDL)
  info_cfg_option(WITH_SDL_DYNLOAD)

  info_cfg_text("")

  message("${_config_msg}")
endif()

if(0)
  print_all_vars()
endif()
  




