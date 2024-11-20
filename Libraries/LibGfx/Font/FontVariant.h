/*
 * Copyright (c) 2018-2020, Andreas Kling <andreas@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Optional.h>
#include <AK/StringView.h>
#include <AK/Vector.h>

#pragma once

namespace Gfx {

using FontFeatureName = StringView;

class FontVariantAlternates {
public:
    bool normal { false };
    bool historical_forms { false };
};

class FontVariantEastAsian {
public:
    enum class Variant { Unset,
        Jis78,
        Jis83,
        Jis90,
        Jis04,
        Simplified,
        Traditional };
    enum class Width { Unset,
        Proportional,
        FullWidth };

    bool normal = false;
    bool ruby = false;
    Variant variant { Variant::Unset };
    Width width { Width::Unset };
};

class FontVariantLigatures {
public:
    enum class Common { Unset,
        Common,
        NoCommon };
    enum class Discretionary { Unset,
        Discretionary,
        NoDiscretionary };
    enum class Historical { Unset,
        Historical,
        NoHistorical };
    enum class Contextual { Unset,
        Contextual,
        NoContextual };

    bool normal = false;
    bool none = false;
    Common common { Common::Unset };
    Discretionary discretionary { Discretionary::Unset };
    Historical historical { Historical::Unset };
    Contextual contextual { Contextual::Unset };
};

class FontVariantNumeric {
public:
    enum class Figure { Unset,
        Lining,
        Oldstyle };
    enum class Spacing { Unset,
        Proportional,
        Tabular };
    enum class Fraction { Unset,
        Diagonal,
        Stacked };

    bool normal = false;
    bool ordinal = false;
    bool slashed_zero = false;
    Figure figure { Figure::Unset };
    Spacing spacing { Spacing::Unset };
    Fraction fraction { Fraction::Unset };
};

extern StringView font_variant_alternates_to_string(FontVariantAlternates);
extern StringView font_variant_east_asian_to_string(FontVariantEastAsian);
extern StringView font_variant_ligatures_to_string(FontVariantLigatures);
extern StringView font_variant_numeric_to_string(FontVariantNumeric);

}
