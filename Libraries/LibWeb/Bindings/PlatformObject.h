/*
 * Copyright (c) 2022, Andreas Kling <andreas@ladybird.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Weakable.h>
#include <LibJS/Runtime/Object.h>
#include <LibWeb/Forward.h>

namespace Web::Bindings {

#define WEB_PLATFORM_OBJECT(class_, base_class)                               \
    JS_OBJECT(class_, base_class)                                             \
    virtual bool implements_interface(String const& interface) const override \
    {                                                                         \
        if (interface == #class_)                                             \
            return true;                                                      \
        return Base::implements_interface(interface);                         \
    }

// https://webidl.spec.whatwg.org/#dfn-platform-object
class PlatformObject : public JS::Object {
    JS_OBJECT(PlatformObject, JS::Object);

public:
    virtual ~PlatformObject() override;

    JS::Realm& realm() const;

    // https://webidl.spec.whatwg.org/#implements
    // This is implemented by overrides that get generated by the WEB_PLATFORM_OBJECT macro.
    [[nodiscard]] virtual bool implements_interface(String const&) const { return false; }

    // ^JS::Object
    virtual JS::ThrowCompletionOr<Optional<JS::PropertyDescriptor>> internal_get_own_property(JS::PropertyKey const&) const override;
    virtual JS::ThrowCompletionOr<bool> internal_set(JS::PropertyKey const&, JS::Value, JS::Value, JS::CacheablePropertyMetadata* = nullptr) override;
    virtual JS::ThrowCompletionOr<bool> internal_define_own_property(JS::PropertyKey const&, JS::PropertyDescriptor const&, Optional<JS::PropertyDescriptor>* precomputed_get_own_property = nullptr) override;
    virtual JS::ThrowCompletionOr<bool> internal_delete(JS::PropertyKey const&) override;
    virtual JS::ThrowCompletionOr<bool> internal_prevent_extensions() override;
    virtual JS::ThrowCompletionOr<GC::MarkedVector<JS::Value>> internal_own_property_keys() const override;

    JS::ThrowCompletionOr<bool> is_named_property_exposed_on_object(JS::PropertyKey const&) const;

protected:
    explicit PlatformObject(JS::Realm&, MayInterfereWithIndexedPropertyAccess = MayInterfereWithIndexedPropertyAccess::No);
    explicit PlatformObject(JS::Object& prototype, MayInterfereWithIndexedPropertyAccess = MayInterfereWithIndexedPropertyAccess::No);

    struct LegacyPlatformObjectFlags {
        u16 supports_indexed_properties : 1 = false;
        u16 supports_named_properties : 1 = false;
        u16 has_indexed_property_setter : 1 = false;
        u16 has_named_property_setter : 1 = false;
        u16 has_named_property_deleter : 1 = false;
        u16 has_legacy_unenumerable_named_properties_interface_extended_attribute : 1 = false;
        u16 has_legacy_override_built_ins_interface_extended_attribute : 1 = false;
        u16 has_global_interface_extended_attribute : 1 = false;
        u16 indexed_property_setter_has_identifier : 1 = false;
        u16 named_property_setter_has_identifier : 1 = false;
        u16 named_property_deleter_has_identifier : 1 = false;
    };
    Optional<LegacyPlatformObjectFlags> m_legacy_platform_object_flags = {};

    enum class IgnoreNamedProps {
        No,
        Yes,
    };
    JS::ThrowCompletionOr<Optional<JS::PropertyDescriptor>> legacy_platform_object_get_own_property(JS::PropertyKey const&, IgnoreNamedProps ignore_named_props) const;

    virtual Optional<JS::Value> item_value(size_t index) const;
    virtual JS::Value named_item_value(FlyString const& name) const;
    virtual Vector<FlyString> supported_property_names() const;
    virtual bool is_supported_property_name(FlyString const&) const;
    bool is_supported_property_index(u32) const;

    // NOTE: These will crash if you make has_named_property_setter return true but do not override these methods.
    // NOTE: This is only used if named_property_setter_has_identifier returns false, otherwise set_value_of_named_property is used instead.
    virtual WebIDL::ExceptionOr<void> set_value_of_new_named_property(String const&, JS::Value);
    virtual WebIDL::ExceptionOr<void> set_value_of_existing_named_property(String const&, JS::Value);

    // NOTE: These will crash if you make has_named_property_setter return true but do not override these methods.
    // NOTE: This is only used if you make named_property_setter_has_identifier return true, otherwise set_value_of_{new,existing}_named_property is used instead.
    virtual WebIDL::ExceptionOr<void> set_value_of_named_property(String const&, JS::Value);

    // NOTE: These will crash if you make has_indexed_property_setter return true but do not override these methods.
    // NOTE: This is only used if indexed_property_setter_has_identifier returns false, otherwise set_value_of_indexed_property is used instead.
    virtual WebIDL::ExceptionOr<void> set_value_of_new_indexed_property(u32, JS::Value);
    virtual WebIDL::ExceptionOr<void> set_value_of_existing_indexed_property(u32, JS::Value);

    // NOTE: These will crash if you make has_named_property_setter return true but do not override these methods.
    // NOTE: This is only used if indexed_property_setter_has_identifier returns true, otherwise set_value_of_{new,existing}_indexed_property is used instead.
    virtual WebIDL::ExceptionOr<void> set_value_of_indexed_property(u32, JS::Value);

    enum class DidDeletionFail {
        // If the named property deleter has an identifier, but does not return a boolean.
        // This is done because we don't know the return type of the deleter outside of the IDL generator.
        NotRelevant,
        No,
        Yes,
    };

    // NOTE: This will crash if you make has_named_property_deleter return true but do not override this method.
    virtual WebIDL::ExceptionOr<DidDeletionFail> delete_value(String const&);

private:
    WebIDL::ExceptionOr<void> invoke_indexed_property_setter(JS::PropertyKey const&, JS::Value);
    WebIDL::ExceptionOr<void> invoke_named_property_setter(FlyString const&, JS::Value);
};

}
