// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_OZONE_PLATFORM_WAYLAND_HOST_XDG_TOPLEVEL_WRAPPER_IMPL_H_
#define UI_OZONE_PLATFORM_WAYLAND_HOST_XDG_TOPLEVEL_WRAPPER_IMPL_H_

#include <xdg-shell-client-protocol.h>
#include <memory>

#include "ui/ozone/platform/wayland/host/shell_toplevel_wrapper.h"

namespace ui {

class XDGSurfaceWrapperImpl;
class WaylandConnection;
class WaylandWindow;

// Toplevel wrapper for xdg-shell stable
class XDGToplevelWrapperImpl : public ShellToplevelWrapper {
 public:
  XDGToplevelWrapperImpl(std::unique_ptr<XDGSurfaceWrapperImpl> surface,
                         WaylandWindow* wayland_window,
                         WaylandConnection* connection);
  XDGToplevelWrapperImpl(const XDGToplevelWrapperImpl&) = delete;
  XDGToplevelWrapperImpl& operator=(const XDGToplevelWrapperImpl&) = delete;
  ~XDGToplevelWrapperImpl() override;

  // ShellToplevelWrapper overrides:
  bool Initialize() override;
  void SetMaximized() override;
  void UnSetMaximized() override;
  void SetFullscreen(WaylandOutput* wayland_output) override;
  void UnSetFullscreen() override;
  void SetMinimized() override;
  void SurfaceMove(WaylandConnection* connection) override;
  void SurfaceResize(WaylandConnection* connection, uint32_t hittest) override;
  void SetTitle(const std::u16string& title) override;
  void AckConfigure(uint32_t serial) override;
  bool IsConfigured() override;
  void SetWindowGeometry(const gfx::Rect& bounds) override;
  void SetMinSize(int32_t width, int32_t height) override;
  void SetMaxSize(int32_t width, int32_t height) override;
  void SetAppId(const std::string& app_id) override;
  void ShowWindowMenu(WaylandConnection* connection,
                      const gfx::Point& point) override;
  void SetDecoration(DecorationMode decoration) override;
  void SetSystemModal(bool modal) override;
  void SetIcon(const gfx::ImageSkia& icon) override;

  XDGToplevelWrapperImpl* AsXDGToplevelWrapper() override;

  XDGSurfaceWrapperImpl* xdg_surface_wrapper() const;

 private:
  friend class WaylandWindowDragController;
  // xdg_toplevel_listener callbacks:
  static void OnToplevelConfigure(void* data,
                                  xdg_toplevel* toplevel,
                                  int32_t width,
                                  int32_t height,
                                  wl_array* states);
  static void OnToplevelClose(void* data, xdg_toplevel* toplevel);
  static void OnConfigureBounds(void* data,
                                xdg_toplevel* toplevel,
                                int32_t width,
                                int32_t height);
  static void OnWmCapabilities(void* data,
                               xdg_toplevel* toplevel,
                               wl_array* capabilities);

  // zxdg_decoration_listener callbacks:
  static void OnDecorationConfigure(void* data,
                                    zxdg_toplevel_decoration_v1* decoration,
                                    uint32_t mode);

  // Send request to wayland compositor to enable a requested decoration mode.
  void SetTopLevelDecorationMode(DecorationMode requested_mode);

  // Initializes the xdg-decoration protocol extension, if available.
  void InitializeXdgDecoration();

  // Creates a wl_region from `shape_rects`.
  wl::Object<wl_region> CreateAndAddRegion(const ShapeRects& shape_rects);

  // Ground surface for this toplevel wrapper.
  std::unique_ptr<XDGSurfaceWrapperImpl> xdg_surface_wrapper_;

  // Non-owing WaylandWindow that uses this toplevel wrapper.
  const raw_ptr<WaylandWindow> wayland_window_;
  const raw_ptr<WaylandConnection> connection_;

  // XDG Shell Stable object.
  wl::Object<xdg_toplevel> xdg_toplevel_;

  wl::Object<zxdg_toplevel_decoration_v1> zxdg_toplevel_decoration_;

  // On client side, it keeps track of the decoration mode currently in
  // use if xdg-decoration protocol extension is available.
  DecorationMode decoration_mode_;
};

}  // namespace ui

#endif  // UI_OZONE_PLATFORM_WAYLAND_HOST_XDG_TOPLEVEL_WRAPPER_IMPL_H_
