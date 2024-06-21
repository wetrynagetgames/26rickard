/*
 * Copyright (c) 2021-2024, Tim Flynn <trflynn89@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Forward.h>
#include <AK/Optional.h>
#include <AK/StringView.h>
#include <AK/Types.h>
#include <LibUnicode/Forward.h>

namespace Unicode {

struct CodePointRange {
    u32 first { 0 };
    u32 last { 0 };
};

struct CodePointRangeComparator {
    constexpr int operator()(u32 code_point, CodePointRange const& range)
    {
        return (code_point > range.last) - (code_point < range.first);
    }
};

Optional<GeneralCategory> general_category_from_string(StringView);
bool code_point_has_general_category(u32 code_point, GeneralCategory general_category);

Optional<Property> property_from_string(StringView);
bool code_point_has_property(u32 code_point, Property property);

bool code_point_has_emoji_property(u32 code_point);
bool code_point_has_emoji_modifier_base_property(u32 code_point);
bool code_point_has_emoji_presentation_property(u32 code_point);
bool code_point_has_identifier_start_property(u32 code_point);
bool code_point_has_identifier_continue_property(u32 code_point);
bool code_point_has_regional_indicator_property(u32 code_point);
bool code_point_has_variation_selector_property(u32 code_point);

bool is_ecma262_property(Property);

Optional<Script> script_from_string(StringView);
bool code_point_has_script(u32 code_point, Script script);
bool code_point_has_script_extension(u32 code_point, Script script);

Optional<BidirectionalClass> bidirectional_class_from_string(StringView);
Optional<BidirectionalClass> bidirectional_class(u32 code_point);

}
