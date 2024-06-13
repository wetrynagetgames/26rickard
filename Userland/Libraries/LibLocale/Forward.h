/*
 * Copyright (c) 2021-2024, Tim Flynn <trflynn89@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Types.h>

namespace Locale {

enum class CalendarPatternStyle : u8;
enum class HourCycle : u8;
enum class Key : u8;
enum class KeywordCalendar : u8;
enum class KeywordCollation : u8;
enum class KeywordColCaseFirst : u8;
enum class KeywordColNumeric : u8;
enum class KeywordHours : u8;
enum class KeywordNumbers : u8;
enum class Locale : u16;
enum class PluralCategory : u8;
enum class Style : u8;
enum class Weekday : u8;

class NumberFormat;

struct CalendarPattern;
struct Keyword;
struct LanguageID;
struct ListFormatPart;
struct LocaleExtension;
struct LocaleID;
struct OtherExtension;
struct PluralOperands;
struct TransformedExtension;
struct TransformedField;

}
