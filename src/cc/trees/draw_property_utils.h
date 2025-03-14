// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TREES_DRAW_PROPERTY_UTILS_H_
#define CC_TREES_DRAW_PROPERTY_UTILS_H_

#include <optional>
#include <vector>

#include "base/memory/raw_ptr.h"
#include "cc/cc_export.h"
#include "cc/layers/layer_collections.h"

namespace gfx {
class Transform;
class Vector2dF;
}  // namespace gfx

namespace cc {

class Layer;
class LayerImpl;
class LayerTreeHost;
class LayerTreeImpl;
class EffectTree;
class TransformTree;
class PropertyTrees;
struct EffectNode;
struct TransformNode;
struct ViewportPropertyIds;

namespace draw_property_utils {

void CC_EXPORT ConcatInverseSurfaceContentsScale(const EffectNode* effect_node,
                                                 gfx::Transform* transform);

// Computes combined (screen space) transforms for every node in the transform
// tree. This must be done prior to calling |ComputeClips|.
void CC_EXPORT
ComputeTransforms(TransformTree* transform_tree,
                  const ViewportPropertyIds& viewport_property_ids);

// Computes screen space opacity for every node in the opacity tree.
void CC_EXPORT ComputeEffects(EffectTree* effect_tree);

void CC_EXPORT UpdatePropertyTrees(LayerTreeHost* layer_tree_host);

void CC_EXPORT
UpdatePropertyTreesAndRenderSurfaces(LayerTreeImpl* layer_tree_impl,
                                     PropertyTrees* property_trees);

void CC_EXPORT FindLayersThatNeedUpdates(LayerTreeHost* layer_tree_host,
                                         LayerList* update_layer_list);

void CC_EXPORT
FindLayersThatNeedUpdates(LayerTreeImpl* layer_tree_impl,
                          std::vector<LayerImpl*>* visible_layer_list);

gfx::Transform CC_EXPORT DrawTransform(const LayerImpl* layer,
                                       const TransformTree& transform_tree,
                                       const EffectTree& effect_tree);

gfx::Transform CC_EXPORT ScreenSpaceTransform(const Layer* layer,
                                              const TransformTree& tree);
gfx::Transform CC_EXPORT ScreenSpaceTransform(const LayerImpl* layer,
                                              const TransformTree& tree);

void CC_EXPORT UpdatePageScaleFactor(PropertyTrees* property_trees,
                                     TransformNode* page_scale_node,
                                     float page_scale_factor);

void CC_EXPORT CalculateDrawProperties(
    LayerTreeImpl* layer_tree_impl,
    RenderSurfaceList* output_render_surface_list,
    LayerImplList* output_update_layer_list_for_testing = nullptr);

bool CC_EXPORT LayerShouldBeSkippedForDrawPropertiesComputation(
    LayerImpl* layer,
    const PropertyTrees* property_trees);

bool CC_EXPORT
IsLayerBackFaceVisibleForTesting(const LayerImpl* layer,
                                 const PropertyTrees* property_trees);

// Returns true if the scales are interchangeable when rasterizing the same
// drawings without visual misalignment.
bool CC_EXPORT RasterScalesApproximatelyEqual(gfx::Vector2dF scale1,
                                              gfx::Vector2dF scale2);

// The result will be used to avoid blurriness of rasterized rendering under
// transforms with subpixel translations. If applicable, the offset will be in
// the range of [0, 1) for each component. After this function returns
// - for a RenderSurfaceImpl, the offset should be subtracted from the the draw
//   transform, and be added to the draw transforms of contributing layers and
//   render surfaces.
// - for a PictureLayerImpl, the offset should be converted to a raster
//   translation which will translate rasterization and offset tex_coord_rect
//   of draw quads.
//
// With this offset applied, the drawings will still be rendered at the same
// screen locations, but the rasterized texels will be aligned to the screen
// pixels, to avoid the additional bluriness during compositing.
//
// Returns nullopt if pixel alignment can't be achieved.
std::optional<gfx::Vector2dF> CC_EXPORT
PixelAlignmentOffset(const gfx::Transform& screen_space_transform,
                     const gfx::Transform& original_draw_transform);

#if DCHECK_IS_ON()
// Checks and logs if double background blur exists in any layers. Returns
// true if no double background blur is detected, false otherwise.
bool CC_EXPORT
LogDoubleBackgroundBlur(const LayerTreeImpl& layer_tree_impl,
                        const RenderSurfaceList& render_surface_list);
#endif
}  // namespace draw_property_utils
}  // namespace cc

#endif  // CC_TREES_DRAW_PROPERTY_UTILS_H_
