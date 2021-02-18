/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2001-2002 by NaN Holding BV.
 * All rights reserved.
 */

/**
 ** OpenGL code is based on the 01-triangles
 ** example from the OpenGL "Red Book" (9th edition)
 ** and support code for loading and compiling shaders.
 ** Note that the 8th edition "Red Book" used GLUT,
 ** and subsequent editions have migrated to GLFW.
 ** Here, we use GHOST to show similarities between those
 ** framework APIs and GHOST.
 **
 ** This "new" code has been shoehorned into gears.c.
 ** Simplification of gears.c has also been made.
 **
 ** OpenGL "Red Book" GitHub URL for 01-triangles example in GLFW:
 ** https://github.com/openglredbook/examples/blob/master/src/01-triangles/01-triangles.cpp
 **
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define FALSE 0

#include "GHOST_C-api.h"

#if defined(WIN32) || defined(__APPLE__)
#  ifdef WIN32
#    include <windows.h>
#    include <GL/gl.h>
#  else /* WIN32 */
/* __APPLE__ is defined */
#    include <AGL/gl.h>
#  endif /* WIN32 */
#else    /* defined(WIN32) || defined(__APPLE__) */
#  include <GL/gl.h>
#endif /* defined(WIN32) || defined(__APPLE__) */

static int sExitRequested = 0;
static GHOST_SystemHandle shSystem = NULL;
static GHOST_WindowHandle sMainWindow = NULL;

#define BUFFER_OFFSET(a) ((void*)(a))

typedef struct {
    GLenum       type;
    const char*  filename;
    GLuint       shader;
} ShaderInfo;

/* ReadShader from "Red Book" */
static const GLchar* ReadShader( const char* filename )
{
#ifdef WIN32
    FILE* infile;
    fopen_s( &infile, filename, "rb" );
#else
    FILE* infile = fopen( filename, "rb" );
#endif // WIN32

    if ( !infile ) {
        return NULL;
    }

    fseek( infile, 0, SEEK_END );
    int len = ftell( infile );
    fseek( infile, 0, SEEK_SET );

    GLchar* source = malloc( sizeof( GLchar ) * ( len + 1 ) );
    fread( source, 1, len, infile );
    fclose( infile );

    source[len] = 0;

    return ( GLchar* )(source);
}

/* LoadShaders from "Red Book" */
GLuint LoadShaders(ShaderInfo* shaders)
{
    if ( shaders == NULL ) {
        return 0;
    }

    GLuint program = glCreateProgram();

    ShaderInfo* entry = shaders;
    while ( entry->type != GL_NONE ) {
        GLuint shader = glCreateShader( entry->type );

        entry->shader = shader;

        const GLchar* source = ReadShader( entry->filename );
        if ( source == NULL ) {
            for ( entry = shaders; entry->type != GL_NONE; ++entry ) {
                glDeleteShader( entry->shader );
                entry->shader = 0;
            }

            return 0;
        }

        glShaderSource( shader, 1, &source, NULL );
        free((void*)source);

        glCompileShader( shader );

        GLint compiled;
        glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
        if ( !compiled ) {
            return 0;
        }

        glAttachShader( program, shader );

        ++entry;
    }

    glLinkProgram( program );

    GLint linked;
    glGetProgramiv( program, GL_LINK_STATUS, &linked );
    if ( !linked ) {
        for ( entry = shaders; entry->type != GL_NONE; ++entry ) {
            glDeleteShader( entry->shader );
            entry->shader = 0;
        }

        return 0;
    }

    return program;
}

//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp BEGIN
//
//////////////////////////////////////////////////////////////////////////////

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer, NumBuffers };
enum Attrib_IDs { vPosition = 0 };

GLuint  VAOs[NumVAOs];
GLuint  Buffers[NumBuffers];

const GLuint  NumVertices = 6;

//----------------------------------------------------------------------------
//
// init
//

void init( void )
{
    glGenVertexArrays( NumVAOs, VAOs );
    glBindVertexArray( VAOs[Triangles] );

    float  vertices[6][2] = {
        { -0.90f, -0.90f }, {  0.85f, -0.90f }, { -0.90f,  0.85f },  // Triangle 1
        {  0.90f, -0.85f }, {  0.90f,  0.90f }, { -0.85f,  0.90f }   // Triangle 2
    };

    glCreateBuffers( NumBuffers, Buffers );
    glBindBuffer( GL_ARRAY_BUFFER, Buffers[ArrayBuffer] );
    glBufferStorage( GL_ARRAY_BUFFER, sizeof(vertices), vertices, 0);

    ShaderInfo shaders[] =
    {
        { GL_VERTEX_SHADER, "./shaders/triangles.vert" },
        { GL_FRAGMENT_SHADER, "./shaders/triangles.frag" },
        { GL_NONE, NULL }
    };

    GLuint program = LoadShaders( shaders );
    glUseProgram( program );

    glVertexAttribPointer( vPosition, 2, GL_FLOAT,
                           GL_FALSE, 0, BUFFER_OFFSET(0) );
    glEnableVertexAttribArray( vPosition );
}

//----------------------------------------------------------------------------
//
// display
//

void display( void )
{
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

    glClearBufferfv(GL_COLOR, 0, black);

    glBindVertexArray( VAOs[Triangles] );
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

//////////////////////////////////////////////////////////////////////////////
//
//  Triangles.cpp END
//
//////////////////////////////////////////////////////////////////////////////


/* The following is simplified code from original gears.c ghost example */

static void setViewPortGL(GHOST_WindowHandle hWindow)
{
    GHOST_RectangleHandle hRect = NULL;
    GLfloat w, h;

    GHOST_ActivateWindowDrawingContext(hWindow);
    hRect = GHOST_GetClientBounds(hWindow);

    w = (float)GHOST_GetWidthRectangle(hRect) / (float)GHOST_GetHeightRectangle(hRect);
    h = 1.0;

    glViewport(0, 0, GHOST_GetWidthRectangle(hRect), GHOST_GetHeightRectangle(hRect));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-w, w, -h, h, 5.0, 60.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -40.0);

    glClearColor(.2f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GHOST_DisposeRectangle(hRect);
}

int processEvent(GHOST_EventHandle hEvent, GHOST_TUserDataPtr userData)
{
    int handled = 1;
    int visibility;
    GHOST_DisplaySetting setting;
    GHOST_WindowHandle window = GHOST_GetEventWindow(hEvent);

    switch (GHOST_GetEventType(hEvent)) {
    case GHOST_kEventWindowClose: {
        GHOST_WindowHandle window = GHOST_GetEventWindow(hEvent);
        if (window == sMainWindow) {
            sExitRequested = 1;
        }
    }
    break;

    case GHOST_kEventWindowActivate:
        handled = 0;
        break;

    case GHOST_kEventWindowDeactivate:
        handled = 0;
        break;

    case GHOST_kEventWindowUpdate: {
        GHOST_WindowHandle window = GHOST_GetEventWindow(hEvent);
        if (!GHOST_ValidWindow(shSystem, window))
            break;
        setViewPortGL(window);
        display();
        GHOST_SwapWindowBuffers(window);
    }
    break;

    default:
        handled = 0;
        break;
    }

    return handled;
}

int main(int argc, char **argv)
{
    GHOST_GLSettings glSettings = {0};
    char *title = "triangles";
    GHOST_EventConsumerHandle consumer = GHOST_CreateEventConsumer(processEvent, NULL);

    /* Create the system */
    shSystem = GHOST_CreateSystem();
    GHOST_AddEventConsumer(shSystem, consumer);

    if (shSystem) {
        /* Create the main window */
        sMainWindow = GHOST_CreateWindow(shSystem,
                                         title,
                                         10,
                                         64,
                                         320,
                                         200,
                                         GHOST_kWindowStateNormal,
                                         GHOST_kDrawingContextTypeOpenGL,
                                         glSettings);


        if (!sMainWindow) {
            printf("could not create main window\n");
            exit(-1);
        }

        init();

        /* Enter main loop */
        while (!sExitRequested) {
            if (!GHOST_ProcessEvents(shSystem, 0)) {
#ifdef WIN32
                /* If there were no events, be nice to other applications */
                Sleep(10);
#endif
            }
            GHOST_DispatchEvents(shSystem);
        }
    }

    /* Dispose windows */
    if (GHOST_ValidWindow(shSystem, sMainWindow)) {
        GHOST_DisposeWindow(shSystem, sMainWindow);
    }

    /* Dispose the system */
    GHOST_DisposeSystem(shSystem);

    return 0;
}
