// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "egl_wrapper.h"
#include "base/logging.h"

typedef NativeDisplayType NativeDisplay;
typedef intptr_t            NativeWindow;
typedef void *            NativePixmap;

typedef struct fbdev_window
{
    unsigned short width;
    unsigned short height;
} fbdev_window;


//rgba8888
/*
static EGLint g_configAttribs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE,
};
  */
static EGLint g_configAttribs[] = {
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE, 5,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE,
};

static EGLDisplay g_EglDisplay = NULL;
static EGLContext g_EglContext = NULL;
static EGLSurface g_EglSurface = NULL;

static NativeDisplayType g_NativeDisplay= EGL_DEFAULT_DISPLAY;
static NativeWindowType g_NativeWindow;

static int g_WindowWidth=0;
static int g_WindowHeigth=0;


NativeDisplay ozone_egl_nativeCreateDisplay(void)
{
    return (NativeDisplay)EGL_DEFAULT_DISPLAY;
}

void ozone_egl_nativeDestroyDisplay(NativeDisplay display)
{
    return;
}

NativeWindow ozone_egl_nativeCreateWindow(const char *title, int width, int height, EGLint visualId)
{
    fbdev_window *fbwin =(fbdev_window *) malloc( sizeof(fbdev_window));
    if (NULL == fbwin)
    {
        return 0;
    }

    fbwin->width  = width;
    fbwin->height = height;
    return (NativeWindow) fbwin;
}

static void ozone_egl_nativeDestroyWindow(NativeWindowType window)
{
    if(window !=0 )
    {
       free((fbdev_window*) window);
    }
}

EGLint ozone_egl_setup(EGLint x, EGLint y, EGLint width, EGLint height )
{
    EGLConfig configs[10];
    EGLint matchingConfigs;
    EGLint err;

    EGLint ctxAttribs[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    g_NativeDisplay = (NativeDisplayType)ozone_egl_nativeCreateDisplay();
    if (g_NativeDisplay < 0)
    {
        LOG(ERROR) << "ozone_egl_nativeCreateDisplay failed!\n";
        return -1;
    }

    g_NativeWindow = (NativeWindowType)ozone_egl_nativeCreateWindow("egl window", width, height, 0);
    
    g_WindowWidth = width;
    g_WindowHeigth = height;

    eglBindAPI(EGL_OPENGL_ES_API);

    /* Get EGLDisplay */
    g_EglDisplay = eglGetDisplay(g_NativeDisplay);

    if (g_EglDisplay == EGL_NO_DISPLAY)
    {
        LOG(ERROR) << "eglGetDisplay returned EGL_NO_DISPLAY";
        return OZONE_EGL_FAILURE;
    }

    if (!eglInitialize(g_EglDisplay, NULL, NULL))
    {
    	LOG(ERROR) << "eglInitialize failed.";
        return OZONE_EGL_FAILURE;
    }

    if (!eglChooseConfig(g_EglDisplay, g_configAttribs, &configs[0],
                         sizeof(configs)/sizeof(configs[0]), &matchingConfigs))
    {
    	LOG(ERROR) << "eglChooseConfig failed.";
        return OZONE_EGL_FAILURE;
    }

    if (matchingConfigs < 1)
    {
    	LOG(ERROR) << "No matching configs found";
        return OZONE_EGL_FAILURE;
    }


    g_EglSurface = eglCreateWindowSurface(g_EglDisplay, configs[0], 0, NULL);
    if (g_EglSurface == NULL)
    {
        LOG(ERROR) << "g_EglSurface == EGL_NO_SURFACE eglGeterror = " << eglGetError();
        return OZONE_EGL_FAILURE;
    }

    g_EglContext = eglCreateContext(g_EglDisplay, configs[0], NULL, ctxAttribs);
    if (g_EglContext == EGL_NO_CONTEXT)
    {
    	LOG(ERROR) << "Failed to get EGL Context";
        return OZONE_EGL_FAILURE;
    }

    eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
    if (EGL_SUCCESS != (err = eglGetError()))
    {
        LOG(ERROR) << "Failed eglMakeCurrent. eglGetError = 0x%x\n" << err;
        return OZONE_EGL_FAILURE;
    }

    return OZONE_EGL_SUCCESS;
}

int ozone_egl_destroy()
{
    int s32Loop = 0;

    /** clean double buffer  **/
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    for (s32Loop = 0; s32Loop < 2; s32Loop++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        ozone_egl_swap();
    }

    eglMakeCurrent(g_EglDisplay, NULL, NULL, NULL);

    if (g_EglContext)
    {
        eglDestroyContext(g_EglDisplay, g_EglContext);
    }

    if (g_EglSurface)
    {
        eglDestroySurface(g_EglDisplay, g_EglSurface);
    }

    eglTerminate(g_EglDisplay);

    ozone_egl_nativeDestroyWindow(g_NativeWindow);
    ozone_egl_nativeDestroyDisplay(g_NativeDisplay);

    return OZONE_EGL_SUCCESS;
}

int ozone_egl_swap()
{
    eglSwapBuffers(g_EglDisplay, g_EglSurface);

    return OZONE_EGL_SUCCESS;
}

NativeDisplayType ozone_egl_getNativedisp()
{
    return g_NativeDisplay;
}

EGLint * ozone_egl_getConfigAttribs()
{
    return g_configAttribs;
}

EGLDisplay ozone_egl_getdisp()
{
    return g_EglDisplay;
}

EGLSurface ozone_egl_getsurface()
{
    return g_EglSurface;
}


void ozone_egl_makecurrent()
{
    eglMakeCurrent(g_EglDisplay, g_EglSurface, g_EglSurface, g_EglContext);
}

GLuint ozone_egl_loadShader ( GLenum type, const char *shaderSrc )
{
   GLuint shader;
   GLint compiled;
   
   // Create the shader object
   shader = glCreateShader ( type );

   if ( shader == 0 )
   	return 0;

   // Load the shader source
   glShaderSource ( shader, 1, &shaderSrc, NULL );
   
   // Compile the shader
   glCompileShader ( shader );

   // Check the compile status
   glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

   if ( !compiled ) 
   {
      GLint infoLen = 0;

      glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = new char[infoLen];

         glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
         printf ( "Error compiling shader:%s\n", infoLog );            
         
         delete infoLog;
      }

      glDeleteShader ( shader );
      return 0;
   }

   return shader;

}
GLuint ozone_egl_loadProgram ( const char *vertShaderSrc, const char *fragShaderSrc )
{
   GLuint vertexShader;
   GLuint fragmentShader;
   GLuint programObject;
   GLint linked;

   // Load the vertex/fragment shaders
   vertexShader = ozone_egl_loadShader ( GL_VERTEX_SHADER, vertShaderSrc );
   if ( vertexShader == 0 )
      return 0;

   fragmentShader = ozone_egl_loadShader ( GL_FRAGMENT_SHADER, fragShaderSrc );
   if ( fragmentShader == 0 )
   {
      glDeleteShader( vertexShader );
      return 0;
   }

   // Create the program object
   programObject = glCreateProgram ( );
   
   if ( programObject == 0 )
      return 0;

   glAttachShader ( programObject, vertexShader );
   glAttachShader ( programObject, fragmentShader );

   // Link the program
   glLinkProgram ( programObject );

   // Check the link status
   glGetProgramiv ( programObject, GL_LINK_STATUS, &linked );

   if ( !linked ) 
   {
      GLint infoLen = 0;

      glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );
      
      if ( infoLen > 1 )
      {
         char* infoLog = new char[infoLen];

         glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
         printf ( "Error linking program:%s\n", infoLog );            
         
         delete infoLog;
      }

      glDeleteProgram ( programObject );
      return 0;
   }

   // Free up no longer needed shader resources
   glDeleteShader ( vertexShader );
   glDeleteShader ( fragmentShader );

   return programObject;
}



int ozone_egl_textureInit (ozone_egl_UserData * userData )
{
   GLbyte vShaderStr[] =  
      "attribute vec4 a_position;   \n"
      "attribute vec2 a_texCoord;   \n"
      "varying vec2 v_texCoord;     \n"
      "void main()                  \n"
      "{                            \n"
      "   gl_Position = a_position; \n"
      "   v_texCoord = a_texCoord;  \n"
      "}                            \n";
   
   GLbyte fShaderStr[] =  
      "precision mediump float;                            \n"
      "varying vec2 v_texCoord;                            \n"
      "uniform sampler2D s_texture;                        \n"
      "void main()                                         \n"
      "{                                                   \n"
      "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
      "}                                                   \n";
      

   // Load the shaders and get a linked program object
   userData->programObject = ozone_egl_loadProgram ( (const char *)vShaderStr, (const char*)fShaderStr );

   // Get the attribute locations
   userData->positionLoc = glGetAttribLocation ( userData->programObject, "a_position" );
   userData->texCoordLoc = glGetAttribLocation ( userData->programObject, "a_texCoord" );
   
   // Get the sampler location
   userData->samplerLoc = glGetUniformLocation ( userData->programObject, "s_texture" );
   
   // Load the texture
   glGenTextures ( 1, &(userData->textureId) );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );
   
   printf("-----glTexImage2D %d %d %d\n",userData->colorType, userData->width,userData->height);
   glTexImage2D ( GL_TEXTURE_2D, 0, userData->colorType, userData->width, userData->height, 0, userData->colorType, GL_UNSIGNED_BYTE, NULL );
   
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
   glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

   glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
   return GL_TRUE;
}


void ozone_egl_textureDraw ( ozone_egl_UserData *userData)
{
                         
   GLfloat vVertices[] = { -0.96f,  0.96f, 0.0f,  // Position 0
                            0.0f,  0.0f,        // TexCoord 0 
                           -0.96f, -0.96f, 0.0f,  // Position 1
                            0.0f,  1.0f,        // TexCoord 1
                            0.96f, -0.96f, 0.0f,  // Position 2
                            1.0f,  1.0f,        // TexCoord 2
                            0.96f,  0.96f, 0.0f,  // Position 3
                            1.0f,  0.0f         // TexCoord 3
                         };                    
                 
   GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
   
   glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, userData->width, userData->height, userData->colorType, GL_UNSIGNED_BYTE, userData->data); 
      
   // Set the viewport
   glViewport ( 0, 0, g_WindowWidth, g_WindowHeigth );
   
   // Clear the color buffer
   glClear ( GL_COLOR_BUFFER_BIT );

   // Use the program object
   glUseProgram ( userData->programObject );

   // Load the vertex position
   glVertexAttribPointer ( userData->positionLoc, 3, GL_FLOAT, 
                           GL_FALSE, 5 * sizeof(GLfloat), vVertices );
   // Load the texture coordinate
   glVertexAttribPointer ( userData->texCoordLoc, 2, GL_FLOAT,
                           GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3] );

   glEnableVertexAttribArray ( userData->positionLoc );
   glEnableVertexAttribArray ( userData->texCoordLoc );

   // Bind the texture
   glActiveTexture ( GL_TEXTURE0 );
   glBindTexture ( GL_TEXTURE_2D, userData->textureId );

   // Set the sampler texture unit to 0
   glUniform1i ( userData->samplerLoc, 0 );

   glDrawElements ( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices );

}


void ozone_egl_textureShutDown ( ozone_egl_UserData *userData )
{
   // Delete texture object
   glDeleteTextures ( 1, &(userData->textureId) );

   // Delete program object
   glDeleteProgram ( userData->programObject );
}
