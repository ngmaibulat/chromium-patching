// Copyright 2016 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_CANVAS_CANVAS2D_BASE_RENDERING_CONTEXT_2D_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_CANVAS_CANVAS2D_BASE_RENDERING_CONTEXT_2D_H_

#include <cstddef>
#include <memory>

#include "base/memory/scoped_refptr.h"
#include "base/notreached.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_canvas_fill_rule.h"
#include "third_party/blink/renderer/bindings/modules/v8/v8_image_smoothing_quality.h"
#include "third_party/blink/renderer/core/html/canvas/canvas_rendering_context.h"
#include "third_party/blink/renderer/core/typed_arrays/dom_typed_array.h"
#include "third_party/blink/renderer/modules/canvas/canvas2d/canvas_2d_recorder_context.h"
#include "third_party/blink/renderer/modules/canvas/canvas2d/canvas_path.h"
#include "third_party/blink/renderer/modules/canvas/canvas2d/canvas_rendering_context_2d_state.h"
#include "third_party/blink/renderer/modules/modules_export.h"
#include "third_party/blink/renderer/platform/graphics/canvas_deferred_paint_record.h"
#include "third_party/blink/renderer/platform/graphics/static_bitmap_image.h"
#include "third_party/blink/renderer/platform/heap/collection_support/heap_hash_map.h"
#include "third_party/blink/renderer/platform/heap/forward.h"  // IWYU pragma: keep (blink::Visitor)
#include "third_party/blink/renderer/platform/heap/member.h"
#include "third_party/blink/renderer/platform/text/layout_locale.h"
#include "third_party/blink/renderer/platform/timer.h"
#include "third_party/blink/renderer/platform/transforms/affine_transform.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/wtf/vector.h"
#include "ui/gfx/geometry/skia_conversions.h"

// IWYU pragma: no_include "third_party/blink/renderer/platform/heap/visitor.h"

class SkPixmap;

namespace base {
class SingleThreadTaskRunner;
}  // namespace base

namespace gfx {
class Rect;
class Vector2d;
}  // namespace gfx

namespace blink {

class Canvas2dGPUTransferOption;
class ExceptionState;
class GPUTexture;
class ImageData;
class ImageDataSettings;
class TextCluster;
class TextClusterOptions;
class TextMetrics;
class V8CanvasFontStretch;
class V8CanvasTextRendering;
class V8CanvasTextAlign;
class V8CanvasTextBaseline;
class V8CanvasDirection;
class V8CanvasFontKerning;
class V8CanvasFontVariantCaps;
class V8GPUTextureFormat;

class MODULES_EXPORT BaseRenderingContext2D : public Canvas2DRecorderContext {
 public:
  static constexpr unsigned kFallbackToCPUAfterReadbacks = 2;

  // Try to restore context 4 times in the event that the context is lost. If
  // the context is unable to be restored after 4 attempts, we discard the
  // backing storage of the context and allocate a new one.
  static const unsigned kMaxTryRestoreContextAttempts = 4;

  BaseRenderingContext2D(const BaseRenderingContext2D&) = delete;
  BaseRenderingContext2D& operator=(const BaseRenderingContext2D&) = delete;

  void ResetInternal() override;

  // https://github.com/WICG/canvas-place-element
  void placeElement(Element* element,
                    double x,
                    double y,
                    ExceptionState& exception_state);
  void OnPlaceElementStateChanged(Element& element);

  ImageData* createImageData(ImageData*, ExceptionState&) const;
  ImageData* createImageData(int sw, int sh, ExceptionState&) const;
  ImageData* createImageData(int sw,
                             int sh,
                             ImageDataSettings*,
                             ExceptionState&) const;

  // For deferred canvases this will have the side effect of drawing recorded
  // commands in order to finalize the frame
  ImageData* getImageData(int sx, int sy, int sw, int sh, ExceptionState&);
  ImageData* getImageData(int sx,
                          int sy,
                          int sw,
                          int sh,
                          ImageDataSettings*,
                          ExceptionState&);
  virtual ImageData* getImageDataInternal(int sx,
                                          int sy,
                                          int sw,
                                          int sh,
                                          ImageDataSettings*,
                                          ExceptionState&);

  void putImageData(ImageData*, int dx, int dy, ExceptionState&);
  void putImageData(ImageData*,
                    int dx,
                    int dy,
                    int dirty_x,
                    int dirty_y,
                    int dirty_width,
                    int dirty_height,
                    ExceptionState&);

  // Transfers a canvas' existing back-buffer to a GPUTexture for use in a
  // WebGPU pipeline. The canvas' image can be used as a texture, or the texture
  // can be bound as a color attachment and modified. After its texture is
  // transferred, the canvas will be reset into an empty, freshly-initialized
  // state.
  GPUTexture* transferToGPUTexture(const Canvas2dGPUTransferOption*,
                                   ExceptionState& exception_state);

  // Replaces the canvas' back-buffer texture with the passed-in GPUTexture.
  // The GPUTexture immediately becomes inaccessible to WebGPU.
  // A GPUValidationError will occur if the GPUTexture is used after
  // `transferBackFromGPUTexture` is called.
  void transferBackFromGPUTexture(ExceptionState& exception_state);

  // Returns the format of the GPUTexture that `transferToGPUTexture` will
  // return. This is useful if you need to create the WebGPU render pipeline
  // before `transferToGPUTexture` is first called.
  V8GPUTextureFormat getTextureFormat() const;

  virtual bool CanCreateCanvas2dResourceProvider() const = 0;

  String lang() const;
  void setLang(const String&);

  V8CanvasDirection direction() const;
  void setDirection(const V8CanvasDirection);

  V8CanvasTextAlign textAlign() const;
  void setTextAlign(const V8CanvasTextAlign);

  V8CanvasTextBaseline textBaseline() const;
  void setTextBaseline(const V8CanvasTextBaseline);

  String letterSpacing() const;
  void setLetterSpacing(const String&);

  String wordSpacing() const;
  void setWordSpacing(const String&);

  V8CanvasTextRendering textRendering() const;
  void setTextRendering(const V8CanvasTextRendering&);

  V8CanvasFontKerning fontKerning() const;
  void setFontKerning(const V8CanvasFontKerning);

  V8CanvasFontStretch fontStretch() const;
  void setFontStretch(const V8CanvasFontStretch&);

  V8CanvasFontVariantCaps fontVariantCaps() const;
  void setFontVariantCaps(const V8CanvasFontVariantCaps&);

  String font() const;
  void setFont(const String& new_font) override;

  void fillText(const String& text, double x, double y);
  void fillText(const String& text, double x, double y, double max_width);
  void strokeText(const String& text, double x, double y);
  void strokeText(const String& text, double x, double y, double max_width);
  TextMetrics* measureText(const String& text);
  // Renders a TextCluster returned by TextMetrics::getTextClusters(). If
  // possible, the align, baseline, and font from the TextCluster will be used.
  // The x and y parameters are added to the values from the TextCluster to
  // position the cluster.
  void fillTextCluster(const TextCluster* text_cluster, double x, double y);
  void fillTextCluster(const TextCluster* text_cluster,
                       double x,
                       double y,
                       const TextClusterOptions* cluster_options);
  void strokeTextCluster(const TextCluster* text_cluster, double x, double y);
  void strokeTextCluster(const TextCluster* text_cluster,
                         double x,
                         double y,
                         const TextClusterOptions* cluster_options);

  void Trace(Visitor*) const override;

  HeapTaskRunnerTimer<BaseRenderingContext2D>
      dispatch_context_lost_event_timer_;
  HeapTaskRunnerTimer<BaseRenderingContext2D>
      dispatch_context_restored_event_timer_;
  HeapTaskRunnerTimer<BaseRenderingContext2D> try_restore_context_event_timer_;
  unsigned try_restore_context_attempt_count_ = 0;

 protected:
  explicit BaseRenderingContext2D(
      scoped_refptr<base::SingleThreadTaskRunner> task_runner);

  virtual UniqueFontSelector* GetFontSelector() const;

  void WillUseCurrentFont() const;
  virtual bool WillSetFont() const;
  virtual bool ResolveFont(const String& new_font) = 0;
  virtual bool CurrentFontResolvedAndUpToDate() const;
  const LayoutLocale* LocaleFromLang();

  virtual bool WritePixels(const SkImageInfo& orig_info,
                           const void* pixels,
                           size_t row_bytes,
                           int x,
                           int y) {
    NOTREACHED();
  }
  virtual scoped_refptr<StaticBitmapImage> GetImage(FlushReason) {
    NOTREACHED();
  }

  virtual void FinalizeFrame(FlushReason) {}

  virtual void DispatchContextLostEvent(TimerBase*);
  virtual void DispatchContextRestoredEvent(TimerBase*);
  virtual void TryRestoreContextEvent(TimerBase*) {}

  static const char kDefaultFont[];
  static const char kInheritString[];

  // Override to prematurely disable acceleration because of a readback.
  // BaseRenderingContext2D automatically disables acceleration after a number
  // of readbacks, this can be overridden to disable acceleration earlier than
  // would typically happen.
  virtual bool ShouldDisableAccelerationBecauseOfReadback() const {
    return false;
  }

  bool context_restorable_{true};

  // TODO(issues.chromium.org/issues/349835587): Add an observer to know if the
  // element is detached and then remove it.
  HeapHashMap<WeakMember<Element>, scoped_refptr<CanvasDeferredPaintRecord>>
      placed_elements_;

 private:
  void DrawTextInternal(const String& text,
                        double x,
                        double y,
                        CanvasRenderingContext2DState::PaintType paint_type,
                        V8CanvasTextAlign align,
                        V8CanvasTextBaseline baseline,
                        unsigned run_start,
                        unsigned run_end,
                        double* max_width = nullptr,
                        const Font* cluster_font = nullptr);

  void PutByteArray(const SkPixmap& source,
                    const gfx::Rect& source_rect,
                    const gfx::Vector2d& dest_offset);
  virtual bool IsCanvas2DBufferValid() const { NOTREACHED(); }

  int num_readbacks_performed_ = 0;
  unsigned read_count_ = 0;
  Member<GPUTexture> webgpu_access_texture_ = nullptr;
  std::unique_ptr<CanvasResourceProvider> resource_provider_from_webgpu_access_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_CANVAS_CANVAS2D_BASE_RENDERING_CONTEXT_2D_H_
