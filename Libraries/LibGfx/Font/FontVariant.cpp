/*
 * Copyright (c) 2024, Johan Dahlin <jdahlin@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <LibGfx/Font/FontVariant.h>

namespace Gfx {

StringView font_variant_alternates_to_string(FontVariantAlternates value)
{
    if (value.normal && value.historical_forms)
        return "normal historical-forms"sv;
    if (value.normal)
        return "normal"sv;
    if (value.historical_forms)
        return "historical-forms"sv;
    return {};
}

StringView font_variant_ligatures_to_string(FontVariantLigatures ligatures)
{
    if (ligatures.normal)
        return "normal"sv;
    if (ligatures.none)
        return "none"sv;

    Vector<StringView> values;
    switch (ligatures.common) {
    case FontVariantLigatures::Common::Common:
        values.append("common-ligatures"sv);
        break;
    case FontVariantLigatures::Common::NoCommon:
        values.append("no-common-ligatures"sv);
        break;
    case FontVariantLigatures::Common::Unset:
        break;
    }

    switch (ligatures.discretionary) {
    case FontVariantLigatures::Discretionary::Discretionary:
        values.append("discretionary-ligatures"sv);
        break;
    case FontVariantLigatures::Discretionary::NoDiscretionary:
        values.append("no-discretionary-ligatures"sv);
        break;
    case FontVariantLigatures::Discretionary::Unset:
        break;
    }

    switch (ligatures.historical) {
    case FontVariantLigatures::Historical::Historical:
        values.append("historical-ligatures"sv);
        break;
    case FontVariantLigatures::Historical::NoHistorical:
        values.append("no-historical-ligatures"sv);
        break;
    case FontVariantLigatures::Historical::Unset:
        break;
    }

    switch (ligatures.contextual) {
    case FontVariantLigatures::Contextual::Contextual:
        values.append("contextual"sv);
        break;
    case FontVariantLigatures::Contextual::NoContextual:
        values.append("no-contextual"sv);
        break;
    case FontVariantLigatures::Contextual::Unset:
        break;
    }

    StringBuilder builder;
    builder.join(' ', values);
    return MUST(builder.to_string());
}

StringView font_variant_east_asian_to_string(FontVariantEastAsian value)
{
    Vector<StringView> values;

    switch (value.variant) {
    case FontVariantEastAsian::Variant::Unset:
        break;
    case FontVariantEastAsian::Variant::Jis78:
        values.append("jis78"sv);
        break;
    case FontVariantEastAsian::Variant::Jis83:
        values.append("jis83"sv);
        break;
    case FontVariantEastAsian::Variant::Jis90:
        values.append("jis90"sv);
        break;
    case FontVariantEastAsian::Variant::Jis04:
        values.append("jis04"sv);
        break;
    case FontVariantEastAsian::Variant::Simplified:
        values.append("simplified"sv);
        break;
    case FontVariantEastAsian::Variant::Traditional:
        values.append("traditional"sv);
        break;
    }

    switch (value.width) {
    case FontVariantEastAsian::Width::Unset:
        break;
    case FontVariantEastAsian::Width::FullWidth:
        values.append("full-width"sv);
        break;
    case FontVariantEastAsian::Width::Proportional:
        values.append("proportional-width"sv);
        break;
    }

    if (value.ruby)
        values.append("ruby"sv);

    StringBuilder builder;
    builder.join(' ', values);
    return MUST(builder.to_string());
}

StringView font_variant_numeric_to_string(FontVariantNumeric value)
{
    Vector<StringView> values;

    if (value.normal)
        values.append("normal"sv);
    if (value.ordinal)
        values.append("ordinal"sv);
    if (value.slashed_zero)
        values.append("slashed-zero"sv);

    switch (value.figure) {
    case FontVariantNumeric::Figure::Unset:
        break;
    case FontVariantNumeric::Figure::Lining:
        values.append("lining-nums"sv);
        break;
    case FontVariantNumeric::Figure::Oldstyle:
        values.append("oldstyle-nums"sv);
        break;
    }

    switch (value.spacing) {
    case FontVariantNumeric::Spacing::Unset:
        break;
    case FontVariantNumeric::Spacing::Proportional:
        values.append("proportional-nums"sv);
        break;
    case FontVariantNumeric::Spacing::Tabular:
        values.append("tabular-nums"sv);
        break;
    }

    switch (value.fraction) {
    case FontVariantNumeric::Fraction::Unset:
        break;
    case FontVariantNumeric::Fraction::Diagonal:
        values.append("diagonal-fractions"sv);
        break;
    case FontVariantNumeric::Fraction::Stacked:
        values.append("stacked-fractions"sv);
        break;
    }

    StringBuilder builder;
    builder.join(' ', values);
    return MUST(builder.to_string());
}

}
