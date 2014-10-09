// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "ui/ozone/platform/egl/egl_surface_factory.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "ui/ozone/public/surface_ozone_egl.h"
#include "ui/ozone/public/surface_ozone_canvas.h"
#include "ui/ozone/public/surface_factory_ozone.h"
#include "ui/gfx/skia_util.h"
#include "ui/gfx/vsync_provider.h"
#include "base/logging.h"

#include "egl_wrapper.h"

#ifndef GL_BGRA_EXT
 #define GL_BGRA_EXT 0x80E1
#endif


#define OZONE_EGL_WINDOW_WIDTH 640
#define OZONE_EGL_WINDOW_HEIGTH 480

namespace ui {

class EglOzoneCanvas: public ui::SurfaceOzoneCanvas {
 public:
  EglOzoneCanvas();
  virtual ~EglOzoneCanvas();
  // SurfaceOzoneCanvas overrides:
  virtual void ResizeCanvas(const gfx::Size& viewport_size) OVERRIDE;
  virtual skia::RefPtr<SkCanvas> GetCanvas() OVERRIDE {
    return skia::SharePtr<SkCanvas>(surface_->getCanvas());
  }
  virtual void PresentCanvas(const gfx::Rect& damage) OVERRIDE;
  
  virtual scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() OVERRIDE {
    return scoped_ptr<gfx::VSyncProvider>();
  }

 private: 
  skia::RefPtr<SkSurface> surface_;
  ozone_egl_UserData userDate_;
};

EglOzoneCanvas::EglOzoneCanvas()
{
    memset(&userDate_,0,sizeof(userDate_));
}
EglOzoneCanvas::~EglOzoneCanvas()
{
    ozone_egl_textureShutDown (&userDate_);
}

void EglOzoneCanvas::ResizeCanvas(const gfx::Size& viewport_size)
{  
  if(userDate_.width == viewport_size.width() && userDate_.height==viewport_size.height())
  {
      return;
  }
  else if(userDate_.width != 0 && userDate_.height !=0)
  {
      ozone_egl_textureShutDown (&userDate_);
  }
  surface_ = skia::AdoptRef(SkSurface::NewRaster(
        SkImageInfo::Make(viewport_size.width(),
                                   viewport_size.height(),
                                   kPMColor_SkColorType,
                                   kPremul_SkAlphaType)));
  userDate_.width = viewport_size.width();
  userDate_.height = viewport_size.height();
  userDate_.colorType = GL_BGRA_EXT;
  ozone_egl_textureInit ( &userDate_);
}

void EglOzoneCanvas::PresentCanvas(const gfx::Rect& damage)
{ 
    SkImageInfo info;
    size_t row_bytes;
    userDate_.data = (char *) surface_->peekPixels(&info, &row_bytes);
    ozone_egl_textureDraw(&userDate_);
    ozone_egl_swap();
}


class OzoneEgl : public ui::SurfaceOzoneEGL {
 public:
  OzoneEgl(gfx::AcceleratedWidget window_id){
     native_window_ = window_id;
  }
  virtual ~OzoneEgl() {
     native_window_=0;
  }

  virtual intptr_t GetNativeWindow() OVERRIDE 
  { 
    return native_window_; 
  }

  virtual bool OnSwapBuffers() OVERRIDE 
  { 
    return true; 
  }

  virtual bool ResizeNativeWindow(const gfx::Size& viewport_size) OVERRIDE {
    return true;
  }


  virtual scoped_ptr<gfx::VSyncProvider> CreateVSyncProvider() OVERRIDE {
    return scoped_ptr<gfx::VSyncProvider>();
  }

 private:
  intptr_t native_window_;
};



SurfaceFactoryEgl::SurfaceFactoryEgl():init_(false)
{

}

SurfaceFactoryEgl::~SurfaceFactoryEgl()
{ 
    DestroySingleWindow(); 
}
  
bool SurfaceFactoryEgl::CreateSingleWindow()
{
  if(init_)
  {
     return true;
  }
  if(!ozone_egl_setup(0, 0, OZONE_EGL_WINDOW_WIDTH, OZONE_EGL_WINDOW_HEIGTH))
  {
      LOG(FATAL) << "CreateSingleWindow";
      return false;
  }
  init_ = true;
  return true;
}

void SurfaceFactoryEgl::DestroySingleWindow() {
  ozone_egl_destroy();
  init_ = false;
}

SurfaceFactoryEgl::HardwareState
SurfaceFactoryEgl::InitializeHardware() {
  return INITIALIZED;
}

void SurfaceFactoryEgl::ShutdownHardware() {
}

intptr_t SurfaceFactoryEgl::GetNativeDisplay() {
  return (intptr_t)ozone_egl_getNativedisp();
}

gfx::AcceleratedWidget SurfaceFactoryEgl::GetAcceleratedWidget() {
  if (!CreateSingleWindow())
    LOG(FATAL) << "failed to create window";
  return (gfx::AcceleratedWidget)GetNativeDisplay();
}

scoped_ptr<ui::SurfaceOzoneEGL>
SurfaceFactoryEgl::CreateEGLSurfaceForWidget(
    gfx::AcceleratedWidget widget) {
  return make_scoped_ptr<ui::SurfaceOzoneEGL>(
      new OzoneEgl(widget));
}

bool SurfaceFactoryEgl::LoadEGLGLES2Bindings(
    AddGLLibraryCallback add_gl_library,
    SetGLGetProcAddressProcCallback set_gl_get_proc_address) { 
  return false;
}

const int32* SurfaceFactoryEgl::GetEGLSurfaceProperties(
    const int32* desired_list) {
  return ozone_egl_getConfigAttribs();
}


scoped_ptr<ui::SurfaceOzoneCanvas> SurfaceFactoryEgl::CreateCanvasForWidget(
      gfx::AcceleratedWidget widget){
  scoped_ptr<EglOzoneCanvas> canvas(new EglOzoneCanvas());
  return canvas.PassAs<ui::SurfaceOzoneCanvas>();
}

}  // namespace ui
