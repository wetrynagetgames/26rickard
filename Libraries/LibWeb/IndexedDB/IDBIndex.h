/*
 * Copyright (c) 2024, stelar7 <dudedbz@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Heap/Heap.h>
#include <LibWeb/Bindings/PlatformObject.h>

namespace Web::IndexedDB {

// https://w3c.github.io/IndexedDB/#index-interface
class IDBIndex : public Bindings::PlatformObject {
    WEB_PLATFORM_OBJECT(IDBIndex, Bindings::PlatformObject);
    JS_DECLARE_ALLOCATOR(IDBIndex);

public:
    virtual ~IDBIndex() override;
    [[nodiscard]] static JS::NonnullGCPtr<IDBIndex> create(JS::Realm&);

protected:
    explicit IDBIndex(JS::Realm&);
    virtual void initialize(JS::Realm&) override;
};

}
