/*
 * Copyright (c) 2024, stelar7 <dudedbz@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibWeb/Bindings/IDBObjectStorePrototype.h>
#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/HTML/EventNames.h>
#include <LibWeb/IndexedDB/IDBObjectStore.h>

namespace Web::IndexedDB {

JS_DEFINE_ALLOCATOR(IDBObjectStore);

IDBObjectStore::~IDBObjectStore() = default;

IDBObjectStore::IDBObjectStore(JS::Realm& realm)
    : PlatformObject(realm)
{
}

JS::NonnullGCPtr<IDBObjectStore> IDBObjectStore::create(JS::Realm& realm)
{
    return realm.heap().allocate<IDBObjectStore>(realm, realm);
}

void IDBObjectStore::initialize(JS::Realm& realm)
{
    Base::initialize(realm);
    WEB_SET_PROTOTYPE_FOR_INTERFACE(IDBObjectStore);
}

}
