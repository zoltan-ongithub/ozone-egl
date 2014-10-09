# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'internal_ozone_platform_deps': [
      'ozone_platform_egl',
    ],
    'internal_ozone_platforms': [
      'egl'
    ],
  },
  'targets': [
    {
      'target_name': 'ozone_platform_egl',
      'type': 'static_library',
      'defines': [
        'OZONE_IMPLEMENTATION',
      ],
      'dependencies': [
        '../../base/base.gyp:base',
        '../events/events.gyp:events',
        '../events/ozone/events_ozone.gyp:events_ozone_evdev',
        '../gfx/gfx.gyp:gfx',
      ],
      'sources': [
        'ozone_platform_egl.cc',
        'ozone_platform_egl.h',
        'egl_surface_factory.cc',
        'egl_surface_factory.h',
        'ozone_platform_egl.h',
        'ozone_platform_egl.cc',
        'egl_wrapper.cc',
        'egl_wrapper.h',
      ],
      'link_settings': {
            'libraries': [
              '-lEGL',
              '-lGLESv2',
            ],
      },
    },
  ],
}
