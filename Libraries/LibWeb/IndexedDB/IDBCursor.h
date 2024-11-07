/*
 * Copyright (c) 2024, stelar7 <dudedbz@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Heap/Heap.h>
#include <LibWeb/Bindings/PlatformObject.h>

namespace Web::IndexedDB {

// https://w3c.github.io/IndexedDB/#cursor-interface
class IDBCursor : public Bindings::PlatformObject {
    WEB_PLATFORM_OBJECT(IDBCursor, Bindings::PlatformObject);
    JS_DECLARE_ALLOCATOR(IDBCursor);

public:
    virtual ~IDBCursor() override;
    [[nodiscard]] static JS::NonnullGCPtr<IDBCursor> create(JS::Realm&);

protected:
    explicit IDBCursor(JS::Realm&);
    virtual void initialize(JS::Realm&) override;
};
}
