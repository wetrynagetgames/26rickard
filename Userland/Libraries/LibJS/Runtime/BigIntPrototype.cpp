/*
 * Copyright (c) 2020-2022, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/Function.h>
#include <AK/TypeCasts.h>
#include <LibJS/Runtime/AbstractOperations.h>
#include <LibJS/Runtime/BigIntObject.h>
#include <LibJS/Runtime/BigIntPrototype.h>
#include <LibJS/Runtime/Completion.h>
#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/GlobalObject.h>
#include <LibJS/Runtime/Intl/NumberFormat.h>
#include <LibJS/Runtime/Intl/NumberFormatConstructor.h>

namespace JS {

BigIntPrototype::BigIntPrototype(Realm& realm)
    : Object(*realm.global_object().object_prototype())
{
}

void BigIntPrototype::initialize(Realm& realm)
{
    auto& vm = this->vm();
    Object::initialize(realm);
    u8 attr = Attribute::Writable | Attribute::Configurable;
    define_native_function(vm.names.toString, to_string, 0, attr);
    define_native_function(vm.names.toLocaleString, to_locale_string, 0, attr);
    define_native_function(vm.names.valueOf, value_of, 0, attr);

    // 21.2.3.5 BigInt.prototype [ @@toStringTag ], https://tc39.es/ecma262/#sec-bigint.prototype-@@tostringtag
    define_direct_property(*vm.well_known_symbol_to_string_tag(), js_string(vm, vm.names.BigInt.as_string()), Attribute::Configurable);
}

// thisBigIntValue ( value ), https://tc39.es/ecma262/#thisbigintvalue
static ThrowCompletionOr<BigInt*> this_bigint_value(VM& vm, Value value)
{
    if (value.is_bigint())
        return &value.as_bigint();
    if (value.is_object() && is<BigIntObject>(value.as_object()))
        return &static_cast<BigIntObject&>(value.as_object()).bigint();
    return vm.throw_completion<TypeError>(ErrorType::NotAnObjectOfType, "BigInt");
}

// 21.2.3.3 BigInt.prototype.toString ( [ radix ] ), https://tc39.es/ecma262/#sec-bigint.prototype.tostring
JS_DEFINE_NATIVE_FUNCTION(BigIntPrototype::to_string)
{
    auto* bigint = TRY(this_bigint_value(vm, vm.this_value()));
    double radix = 10;
    if (!vm.argument(0).is_undefined()) {
        radix = TRY(vm.argument(0).to_integer_or_infinity(vm));
        if (radix < 2 || radix > 36)
            return vm.throw_completion<RangeError>(ErrorType::InvalidRadix);
    }
    return js_string(vm, bigint->big_integer().to_base(radix));
}

// 21.2.3.2 BigInt.prototype.toLocaleString ( [ reserved1 [ , reserved2 ] ] ), https://tc39.es/ecma262/#sec-bigint.prototype.tolocalestring
// 19.3.1 BigInt.prototype.toLocaleString ( [ locales [ , options ] ] ), https://tc39.es/ecma402/#sup-bigint.prototype.tolocalestring
JS_DEFINE_NATIVE_FUNCTION(BigIntPrototype::to_locale_string)
{
    auto& realm = *vm.current_realm();

    auto locales = vm.argument(0);
    auto options = vm.argument(1);

    // 1. Let x be ? thisBigIntValue(this value).
    auto* bigint = TRY(this_bigint_value(vm, vm.this_value()));

    // 2. Let numberFormat be ? Construct(%NumberFormat%, « locales, options »).
    auto* number_format = static_cast<Intl::NumberFormat*>(TRY(construct(vm, *realm.global_object().intl_number_format_constructor(), locales, options)));

    // 3. Return ? FormatNumeric(numberFormat, x).
    auto formatted = Intl::format_numeric(vm, *number_format, Value(bigint));
    return js_string(vm, move(formatted));
}

// 21.2.3.4 BigInt.prototype.valueOf ( ), https://tc39.es/ecma262/#sec-bigint.prototype.valueof
JS_DEFINE_NATIVE_FUNCTION(BigIntPrototype::value_of)
{
    return TRY(this_bigint_value(vm, vm.this_value()));
}

}
