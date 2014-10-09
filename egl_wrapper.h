// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef UI_OZONE_EGL_WRAPPER_H_
#define UI_OZONE_EGL_WRAPPER_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>

#define OZONE_EGL_SUCCESS 1
#define OZONE_EGL_FAILURE 0

#define GLCheckError() \
    {                                                                \
        GLint err = glGetError();                                    \
        if (err != GL_NO_ERROR)                                      \
        {                                                            \
            fprintf(stderr, "\nfunction: %s, line: %d, err: 0x%x\n", \
                    __FUNCTION__, __LINE__, err);                              \
            return HI_FAILURE;                                       \
        }                                                            \
    }

#define EGLCheckError() \
    {                                                                \
        GLint err = eglGetError();                                   \
        if (err != EGL_SUCCESS)                                      \
        {                                                            \
            fprintf(stderr, "\nfunction: %s, line: %d, err: 0x%x\n", \
                    __FUNCTION__, __LINE__, err);                              \
            return HI_FAILURE;                                       \
        }                                                            \
    }


typedef struct
{
   // Handle to a program object
   GLuint programObject;

   // Attribute locations
   GLint  positionLoc;
   GLint  texCoordLoc;

   // Sampler location
   GLint samplerLoc;

   // Texture handle
   GLuint textureId;
   
   GLint colorType;
   GLint width;
   GLint height;
   char * data;

} ozone_egl_UserData;


EGLint ozone_egl_setup(EGLint x, EGLint y, EGLint width, EGLint height );
int     ozone_egl_destroy();
int     ozone_egl_swap();
NativeDisplayType ozone_egl_getNativedisp();
EGLint * ozone_egl_getConfigAttribs();
EGLDisplay ozone_egl_getdisp();
EGLSurface ozone_egl_getsurface();
void ozone_egl_makecurrent();
int ozone_egl_textureInit (ozone_egl_UserData * userData );
void ozone_egl_textureDraw ( ozone_egl_UserData *userData );
void ozone_egl_textureShutDown ( ozone_egl_UserData *userData );

#endif
