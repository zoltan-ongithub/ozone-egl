// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_SURFACE_FACTORY_H_
#define UI_OZONE_PLATFORM_SURFACE_FACTORY_H_

#include "base/memory/scoped_ptr.h"
#include "ui/ozone/public/surface_factory_ozone.h"

namespace gfx {
class SurfaceOzone;
}

namespace ui {

class SurfaceFactoryEgl : public ui::SurfaceFactoryOzone {
 public:
  SurfaceFactoryEgl();
  virtual ~SurfaceFactoryEgl();

  // Create the window.
  bool CreateSingleWindow();
  void DestroySingleWindow();

  // SurfaceFactoryOzone:
  virtual HardwareState InitializeHardware() OVERRIDE;
  virtual void ShutdownHardware() OVERRIDE;
  virtual intptr_t GetNativeDisplay() OVERRIDE;
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;
  virtual scoped_ptr<ui::SurfaceOzoneEGL> CreateEGLSurfaceForWidget(
      gfx::AcceleratedWidget widget) OVERRIDE;
  virtual const int32* GetEGLSurfaceProperties(
      const int32* desired_list) OVERRIDE;
  virtual bool LoadEGLGLES2Bindings(
      AddGLLibraryCallback add_gl_library,
      SetGLGetProcAddressProcCallback set_gl_get_proc_address) OVERRIDE;
  virtual scoped_ptr<ui::SurfaceOzoneCanvas> CreateCanvasForWidget(
      gfx::AcceleratedWidget widget) OVERRIDE;

 private:
    bool init_;
};

}  // namespace ui

#endif
