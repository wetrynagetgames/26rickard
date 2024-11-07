/*
 * Copyright (c) 2024, Andreas Kling <andreas@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define AK_DONT_REPLACE_STD

#include <AK/OwnPtr.h>
#include <LibGfx/PainterSkia.h>
#include <LibGfx/PathSkia.h>

#include <AK/TypeCasts.h>
#include <core/SkBitmap.h>
#include <core/SkBlurTypes.h>
#include <core/SkCanvas.h>
#include <core/SkColorFilter.h>
#include <core/SkMaskFilter.h>
#include <core/SkPath.h>
#include <core/SkPathBuilder.h>
#include <core/SkRRect.h>
#include <core/SkSurface.h>
#include <effects/SkGradientShader.h>
#include <effects/SkImageFilters.h>
#include <gpu/GrDirectContext.h>
#include <gpu/ganesh/SkSurfaceGanesh.h>
#include <pathops/SkPathOps.h>

namespace Gfx {

static SkColorType to_skia_color_type(Gfx::BitmapFormat format)
{
    switch (format) {
    case Gfx::BitmapFormat::Invalid:
        return kUnknown_SkColorType;
    case Gfx::BitmapFormat::BGRA8888:
    case Gfx::BitmapFormat::BGRx8888:
        return kBGRA_8888_SkColorType;
    case Gfx::BitmapFormat::RGBA8888:
        return kRGBA_8888_SkColorType;
    case Gfx::BitmapFormat::RGBx8888:
        return kRGB_888x_SkColorType;
    default:
        return kUnknown_SkColorType;
    }
}

static SkAlphaType to_skia_alpha_type(Gfx::AlphaType alpha_type)
{
    switch (alpha_type) {
    case AlphaType::Premultiplied:
        return kPremul_SkAlphaType;
    case AlphaType::Unpremultiplied:
        return kUnpremul_SkAlphaType;
    default:
        VERIFY_NOT_REACHED();
    }
}

struct PainterSkia::Impl {
    NonnullRefPtr<Gfx::Bitmap> gfx_bitmap;
    OwnPtr<SkBitmap> sk_bitmap;
    OwnPtr<SkCanvas> sk_canvas;

    Impl(NonnullRefPtr<Gfx::Bitmap> target_bitmap)
        : gfx_bitmap(move(target_bitmap))
    {
        sk_bitmap = make<SkBitmap>();
        SkImageInfo info = SkImageInfo::Make(gfx_bitmap->width(), gfx_bitmap->height(), to_skia_color_type(gfx_bitmap->format()), to_skia_alpha_type(gfx_bitmap->alpha_type()));
        sk_bitmap->installPixels(info, gfx_bitmap->scanline(0), gfx_bitmap->pitch());

        sk_canvas = make<SkCanvas>(*sk_bitmap);
    }

    SkCanvas* canvas() { return sk_canvas; }
};

static constexpr SkRect to_skia_rect(auto const& rect)
{
    return SkRect::MakeXYWH(rect.x(), rect.y(), rect.width(), rect.height());
}

static constexpr SkColor to_skia_color(Gfx::Color const& color)
{
    return SkColorSetARGB(color.alpha(), color.red(), color.green(), color.blue());
}

static constexpr SkBlendMode to_skia_blend_mode(Gfx::BlendMode blend_mode) {
    switch (blend_mode) {
    case BlendMode::Clear:
        return SkBlendMode::kClear;
    case BlendMode::Src:
        return SkBlendMode::kSrc;
    case BlendMode::Dst:
        return SkBlendMode::kDst;
    case BlendMode::SrcOver:
        return SkBlendMode::kSrcOver;
    case BlendMode::DstOver:
        return SkBlendMode::kDstOver;
    case BlendMode::SrcIn:
        return SkBlendMode::kSrcIn;
    case BlendMode::DstIn:
        return SkBlendMode::kDstIn;
    case BlendMode::SrcOut:
        return SkBlendMode::kSrcOut;
    case BlendMode::DstOut:
        return SkBlendMode::kDstOut;
    case BlendMode::SrcATop:
        return SkBlendMode::kSrcATop;
    case BlendMode::DstATop:
        return SkBlendMode::kDstATop;
    case BlendMode::Xor:
        return SkBlendMode::kXor;
    case BlendMode::Plus:
        return SkBlendMode::kPlus;
    case BlendMode::Modulate:
        return SkBlendMode::kModulate;
    case BlendMode::Screen:
        return SkBlendMode::kScreen;
    case BlendMode::Overlay:
        return SkBlendMode::kOverlay;
    case BlendMode::Darken:
        return SkBlendMode::kDarken;
    case BlendMode::Lighten:
        return SkBlendMode::kLighten;
    case BlendMode::ColorDodge:
        return SkBlendMode::kColorDodge;
    case BlendMode::ColorBurn:
        return SkBlendMode::kColorBurn;
    case BlendMode::HardLight:
        return SkBlendMode::kHardLight;
    case BlendMode::SoftLight:
        return SkBlendMode::kSoftLight;
    case BlendMode::Difference:
        return SkBlendMode::kDifference;
    case BlendMode::Exclusion:
        return SkBlendMode::kExclusion;
    case BlendMode::Multiply:
        return SkBlendMode::kMultiply;
    case BlendMode::Hue:
        return SkBlendMode::kHue;
    case BlendMode::Saturation:
        return SkBlendMode::kSaturation;
    case BlendMode::Color:
        return SkBlendMode::kColor;
    case BlendMode::Luminosity:
        return SkBlendMode::kLuminosity;
    }
    VERIFY_NOT_REACHED();
}

static SkPath to_skia_path(Gfx::Path const& path)
{
    return static_cast<PathImplSkia const&>(path.impl()).sk_path();
}

static SkPathFillType to_skia_path_fill_type(Gfx::WindingRule winding_rule)
{
    switch (winding_rule) {
    case Gfx::WindingRule::Nonzero:
        return SkPathFillType::kWinding;
    case Gfx::WindingRule::EvenOdd:
        return SkPathFillType::kEvenOdd;
    }
    VERIFY_NOT_REACHED();
}

PainterSkia::PainterSkia(NonnullRefPtr<Gfx::Bitmap> target_bitmap)
    : m_impl(adopt_own(*new Impl { move(target_bitmap) }))
{
}

PainterSkia::~PainterSkia() = default;

static SkSamplingOptions to_skia_sampling_options(Gfx::ScalingMode scaling_mode)
{
    switch (scaling_mode) {
    case Gfx::ScalingMode::NearestNeighbor:
        return SkSamplingOptions(SkFilterMode::kNearest);
    case Gfx::ScalingMode::BilinearBlend:
    case Gfx::ScalingMode::SmoothPixels:
        return SkSamplingOptions(SkFilterMode::kLinear);
    case Gfx::ScalingMode::BoxSampling:
        return SkSamplingOptions(SkCubicResampler::Mitchell());
    default:
        VERIFY_NOT_REACHED();
    }
}


void PainterSkia::set_transform(Gfx::AffineTransform const& transform)
{
    auto matrix = SkMatrix::MakeAll(
        transform.a(), transform.c(), transform.e(),
        transform.b(), transform.d(), transform.f(),
        0, 0, 1);

    impl().canvas()->setMatrix(matrix);
}


static SkPoint to_skia_point(auto const& point)
{
    return SkPoint::Make(point.x(), point.y());
}

static SkPaint to_skia_paint(Gfx::PaintStyle const& style, Gfx::FloatRect const& bounding_rect)
{
    if (is<Gfx::CanvasLinearGradientPaintStyle>(style)) {
        auto const& linear_gradient = static_cast<Gfx::CanvasLinearGradientPaintStyle const&>(style);
        auto const& color_stops = linear_gradient.color_stops();

        SkPaint paint;
        Vector<SkColor> colors;
        colors.ensure_capacity(color_stops.size());
        Vector<SkScalar> positions;
        positions.ensure_capacity(color_stops.size());
        for (auto const& color_stop : color_stops) {
            colors.append(to_skia_color(color_stop.color));
            positions.append(color_stop.position);
        }

        Array<SkPoint, 2> points;
        points[0] = to_skia_point(linear_gradient.start_point());
        points[1] = to_skia_point(linear_gradient.end_point());

        SkMatrix matrix;
        auto shader = SkGradientShader::MakeLinear(points.data(), colors.data(), positions.data(), color_stops.size(), SkTileMode::kClamp, 0, &matrix);
        paint.setShader(shader);
        return paint;
    }

    if (is<Gfx::CanvasRadialGradientPaintStyle>(style)) {
        auto const& radial_gradient = static_cast<Gfx::CanvasRadialGradientPaintStyle const&>(style);
        auto const& color_stops = radial_gradient.color_stops();

        SkPaint paint;
        Vector<SkColor> colors;
        colors.ensure_capacity(color_stops.size());
        Vector<SkScalar> positions;
        positions.ensure_capacity(color_stops.size());
        for (auto const& color_stop : color_stops) {
            colors.append(to_skia_color(color_stop.color));
            positions.append(color_stop.position);
        }

        auto start_center = radial_gradient.start_center();
        auto end_center = radial_gradient.end_center();
        auto start_radius = radial_gradient.start_radius();
        auto end_radius = radial_gradient.end_radius();

        start_center.translate_by(bounding_rect.location());
        end_center.translate_by(bounding_rect.location());

        auto start_sk_point = to_skia_point(start_center);
        auto end_sk_point = to_skia_point(end_center);

        SkMatrix matrix;
        auto shader = SkGradientShader::MakeTwoPointConical(start_sk_point, start_radius, end_sk_point, end_radius, colors.data(), positions.data(), color_stops.size(), SkTileMode::kClamp, 0, &matrix);
        paint.setShader(shader);
    }
    return {};
}

static bool requires_clear_outside_source(Gfx::BlendMode global_composite_operation) {
    switch (global_composite_operation) {
    case Gfx::BlendMode::Src:
    case Gfx::BlendMode::SrcIn:
    case Gfx::BlendMode::DstIn:
    case Gfx::BlendMode::SrcOut:
    case Gfx::BlendMode::DstATop:
        return true;
    default:
        return false;
    }
}

static void clear_outside_source_if_needed_fill(SkPath const& source_path, SkCanvas& canvas, Gfx::BlendMode global_composite_operation) {
    // NOTE: Some blending modes need to clear the area outside the source, skia does not do this on its own.
    if (!requires_clear_outside_source(global_composite_operation))
        return;

    SkPaint paint;
    const SkRect size_rect = SkRect::Make(canvas.getBaseLayerSize());
    paint.setAntiAlias(true);
    paint.setBlendMode(SkBlendMode::kClear);
    canvas.save();
    canvas.clipPath(source_path, SkClipOp::kDifference, true);
    canvas.drawRect(size_rect, paint);
    canvas.restore();
}

void PainterSkia::clear_rect(Gfx::FloatRect const& rect, Gfx::Color color)
{
    SkPaint paint;
    paint.setColor(to_skia_color(color));
    paint.setBlendMode(SkBlendMode::kClear);
    impl().canvas()->drawRect(to_skia_rect(rect), paint);
}

void PainterSkia::draw_bitmap(Gfx::FloatRect const& dst_rect, Gfx::Bitmap const& src_bitmap, Gfx::IntRect const& src_rect, Gfx::ScalingMode scaling_mode, float global_alpha, Gfx::BlendMode global_composite_operation)
{
    auto skia_src_rect = to_skia_rect(src_rect);
    clear_outside_source_if_needed_fill(SkPath::Rect(skia_src_rect), *impl().canvas(), global_composite_operation);

    SkBitmap sk_bitmap;
    SkImageInfo info = SkImageInfo::Make(src_bitmap.width(), src_bitmap.height(), to_skia_color_type(src_bitmap.format()), to_skia_alpha_type(src_bitmap.alpha_type()));
    sk_bitmap.installPixels(info, const_cast<void*>(static_cast<void const*>(src_bitmap.scanline(0))), src_bitmap.pitch());

    SkPaint paint;
    paint.setAlpha(static_cast<u8>(global_alpha * 255));
    paint.setBlendMode(to_skia_blend_mode(global_composite_operation));

    impl().canvas()->drawImageRect(
        sk_bitmap.asImage(),
        skia_src_rect,
        to_skia_rect(dst_rect),
        to_skia_sampling_options(scaling_mode),
        &paint,
        SkCanvas::kStrict_SrcRectConstraint);
}

void PainterSkia::stroke_path(Gfx::Path const& path, Gfx::Color color, float thickness, Gfx::BlendMode blend_mode)
{
    // Skia treats zero thickness as a special case and will draw a hairline, while we want to draw nothing.
    if (!thickness)
        return;

    // FIXME: Some blending modes need to clear the region outside the drawn path, Skia doesn't do it on it's onw.
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(thickness);
    paint.setColor(to_skia_color(color));
    paint.setBlendMode(to_skia_blend_mode(blend_mode));
    auto sk_path = to_skia_path(path);
    impl().canvas()->drawPath(sk_path, paint);
}
void PainterSkia::stroke_path(Gfx::Path const& path, Gfx::PaintStyle const& paint_style, float thickness, float global_alpha, Gfx::BlendMode global_composite_operation)
{
    // Skia treats zero thickness as a special case and will draw a hairline, while we want to draw nothing.
    if (!thickness)
        return;

    // FIXME: Some blending modes need to clear the region outside the drawn path, Skia doesn't do it on it's onw.
    auto sk_path = to_skia_path(path);
    auto paint = to_skia_paint(paint_style, path.bounding_box());
    paint.setAntiAlias(true);
    paint.setAlphaf(global_alpha);
    paint.setBlendMode(to_skia_blend_mode(global_composite_operation));
    paint.setStyle(SkPaint::Style::kStroke_Style);
    paint.setStrokeWidth(thickness);
    impl().canvas()->drawPath(sk_path, paint);
}

void PainterSkia::fill_path(Gfx::Path const& path, Gfx::Color color, Gfx::WindingRule winding_rule, Gfx::BlendMode global_composite_operation)
{
    auto sk_path = to_skia_path(path);
    sk_path.setFillType(to_skia_path_fill_type(winding_rule));
    clear_outside_source_if_needed_fill(sk_path, *impl().canvas(), global_composite_operation);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(to_skia_color(color));
    paint.setBlendMode(to_skia_blend_mode(global_composite_operation));
    impl().canvas()->drawPath(sk_path, paint);
}

void PainterSkia::fill_path(Gfx::Path const& path, Gfx::PaintStyle const& paint_style, float global_alpha, Gfx::BlendMode global_composite_operation, Gfx::WindingRule winding_rule)
{
    auto sk_path = to_skia_path(path);
    sk_path.setFillType(to_skia_path_fill_type(winding_rule));
    clear_outside_source_if_needed_fill(sk_path, *impl().canvas(), global_composite_operation);

    auto paint = to_skia_paint(paint_style, path.bounding_box());
    paint.setAntiAlias(true);
    paint.setAlphaf(global_alpha);
    paint.setBlendMode(to_skia_blend_mode(global_composite_operation));
    impl().canvas()->drawPath(sk_path, paint);
}

void PainterSkia::save()
{
    impl().canvas()->save();
}

void PainterSkia::restore()
{
    impl().canvas()->restore();
}

void PainterSkia::clip(Gfx::Path const& path, Gfx::WindingRule winding_rule)
{
    auto sk_path = to_skia_path(path);
    sk_path.setFillType(to_skia_path_fill_type(winding_rule));
    impl().canvas()->clipPath(sk_path, SkClipOp::kIntersect, true);
}

}
