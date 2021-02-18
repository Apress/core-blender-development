Chapter 3 consists of a "minimal" blender source tree 
(see directory tree below). It includes only dependencies needed
by ghost. 

It should be possible to prune further. However, that
would require modifying ghost itself, to remove its dependency on
"core" modules -- i.e., those modules provided in chapter_3/source/.

This stripped-down blender source tree delineates "core" blender modules
from ghost. You may also use this to build applications, other than
blender, leveraging ghost.

An example of this has been added to chapter_3/intern/ghost/test/triangles/.
More info can be found in the comments from GHOST_C-Triangles.c.

To build, use chapter_3/intern/ghost/test/CMakeLists.txt. Note that this 
CMakeLists.txt has been modified to include the following cmake script excerpt.

# Triangles (C)
add_executable(triangles_c
 ${CMAKE_SOURCE_DIR}/triangles/GHOST_C-Triangles.c)
    
add_custom_command(TARGET triangles_c PRE_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_SOURCE_DIR}/triangles/shaders/ $<TARGET_FILE_DIR:triangles_c>/shaders/)

target_link_libraries(triangles_c
    ghost_lib
    glewmx_lib
    string_lib
    ${OPENGL_gl_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${PLATFORM_LINKLIBS}
    )
    
This additional script is in part responsible for copying the shaders 
subdirectory, and its contents, to the build directory specified in the
cmake configuration step.

Chapter 3's directory structure:

chapter_3/
|-- build_files
|   `-- cmake
|       |-- Modules
|       `-- platform
|-- extern
|   |-- glew
|   |   |-- include
|   |   |   `-- GL
|   |   `-- src
|   |-- glew-es
|   |   |-- include
|   |   |   `-- GL
|   |   `-- src
|   |-- sdlew
|   |   |-- include
|   |   |   `-- SDL2
|   |   `-- src
|   |-- wcwidth
|   `-- xdnd
|-- intern
|   |-- atomic
|   |   `-- intern
|   |-- ghost
|   |   |-- intern
|   |   `-- test
|   |       |-- gears
|   |       `-- triangles <-- "triangles" example ported to ghost 
|   |           `-- shaders
|   |-- glew-mx
|   |   `-- intern
|   |-- guardedalloc
|   |   |-- cpp
|   |   |-- intern
|   |   `-- test
|   |       `-- simpletest
|   |-- libc_compat
|   `-- string
|       `-- intern
`-- source
    `-- blender
        |-- blenlib
        |   `-- intern
        |-- imbuf
        |   `-- intern
        |       |-- cineon
        |       |-- dds
        |       |-- oiio
        |       `-- openexr
        `-- makesdna
            `-- intern
