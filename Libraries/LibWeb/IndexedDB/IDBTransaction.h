/*
 * Copyright (c) 2024, stelar7 <dudedbz@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Heap/GCPtr.h>
#include <LibWeb/Bindings/IDBDatabasePrototype.h>
#include <LibWeb/Bindings/IDBTransactionPrototype.h>
#include <LibWeb/DOM/Event.h>
#include <LibWeb/DOM/EventTarget.h>
#include <LibWeb/IndexedDB/IDBDatabase.h>

namespace Web::IndexedDB {

// https://w3c.github.io/IndexedDB/#transaction
class IDBTransaction : public DOM::EventTarget {
    WEB_PLATFORM_OBJECT(IDBTransaction, DOM::EventTarget);
    JS_DECLARE_ALLOCATOR(IDBTransaction);

    enum TransactionState {
        Active,
        Inactive,
        Committing,
        Finished
    };

public:
    virtual ~IDBTransaction() override;

    [[nodiscard]] static JS::NonnullGCPtr<IDBTransaction> create(JS::Realm&, JS::NonnullGCPtr<IDBDatabase>);
    [[nodiscard]] Bindings::IDBTransactionMode mode() const { return m_mode; }
    [[nodiscard]] TransactionState state() const { return m_state; }
    [[nodiscard]] JS::GCPtr<WebIDL::DOMException> error() const { return m_error; }
    [[nodiscard]] JS::NonnullGCPtr<IDBDatabase> connection() const { return m_connection; }
    [[nodiscard]] Bindings::IDBTransactionDurability durability() const { return m_durability; }

    void set_mode(Bindings::IDBTransactionMode mode) { m_mode = mode; }
    void set_state(TransactionState state) { m_state = state; }
    void set_error(JS::GCPtr<WebIDL::DOMException> error) { m_error = error; }

    [[nodiscard]] bool is_upgrade_transaction() const { return m_mode == Bindings::IDBTransactionMode::Versionchange; }
    [[nodiscard]] bool is_readonly() const { return m_mode == Bindings::IDBTransactionMode::Readonly; }
    [[nodiscard]] bool is_readwrite() const { return m_mode == Bindings::IDBTransactionMode::Readwrite; }

    void set_onabort(WebIDL::CallbackType*);
    WebIDL::CallbackType* onabort();
    void set_oncomplete(WebIDL::CallbackType*);
    WebIDL::CallbackType* oncomplete();
    void set_onerror(WebIDL::CallbackType*);
    WebIDL::CallbackType* onerror();

protected:
    explicit IDBTransaction(JS::Realm&, JS::NonnullGCPtr<IDBDatabase>);
    virtual void initialize(JS::Realm&) override;
    virtual void visit_edges(Visitor& visitor) override;

private:
    JS::NonnullGCPtr<IDBDatabase> m_connection;
    Bindings::IDBTransactionMode m_mode;
    Bindings::IDBTransactionDurability m_durability { Bindings::IDBTransactionDurability::Default };
    TransactionState m_state;
    JS::GCPtr<WebIDL::DOMException> m_error;
};
}
