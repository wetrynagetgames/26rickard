/*
 * Copyright (c) 2024, stelar7 <dudedbz@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibJS/Runtime/VM.h>
#include <LibWeb/DOM/EventDispatcher.h>
#include <LibWeb/HTML/EventNames.h>
#include <LibWeb/IndexedDB/IDBDatabase.h>
#include <LibWeb/IndexedDB/IDBRequest.h>
#include <LibWeb/IndexedDB/IDBTransaction.h>
#include <LibWeb/IndexedDB/IDBVersionChangeEvent.h>
#include <LibWeb/IndexedDB/Internal/Algorithms.h>
#include <LibWeb/IndexedDB/Internal/ConnectionQueueHandler.h>
#include <LibWeb/IndexedDB/Internal/Database.h>
#include <LibWeb/StorageAPI/StorageKey.h>

namespace Web::IndexedDB {

// https://w3c.github.io/IndexedDB/#open-a-database-connection
WebIDL::ExceptionOr<JS::NonnullGCPtr<IDBDatabase>> open_a_database_connection(JS::Realm& realm, StorageAPI::StorageKey storage_key, String name, Optional<u64> maybe_version, JS::NonnullGCPtr<IDBRequest> request)
{
    // 1. Let queue be the connection queue for storageKey and name.
    auto& queue = ConnectionQueueHandler::for_key_and_name(storage_key, name);

    // 2. Add request to queue.
    queue.append(request);

    // 3. Wait until all previous requests in queue have been processed.
    HTML::main_thread_event_loop().spin_until(JS::create_heap_function(realm.vm().heap(), [queue, request]() {
        return queue.all_previous_requests_processed(request);
    }));

    // 4. Let db be the database named name in storageKey, or null otherwise.
    JS::GCPtr<Database> db;
    auto maybe_db = Database::for_key_and_name(storage_key, name);
    if (maybe_db.has_value()) {
        db = maybe_db.value();
    }

    // 5. If version is undefined, let version be 1 if db is null, or db’s version otherwise.
    auto version = maybe_version.value_or(maybe_db.has_value() ? maybe_db.value()->version() : 1);

    // 6. If db is null, let db be a new database with name name, version 0 (zero), and with no object stores.
    // If this fails for any reason, return an appropriate error (e.g. a "QuotaExceededError" or "UnknownError" DOMException).
    if (!maybe_db.has_value()) {
        auto maybe_database = Database::create_for_key_and_name(realm, storage_key, name);

        if (maybe_database.is_error()) {
            return WebIDL::OperationError::create(realm, "Unable to create a new database"_string);
        }

        db = maybe_database.release_value();
    }

    // 7. If db’s version is greater than version, return a newly created "VersionError" DOMException and abort these steps.
    if (db->version() > version) {
        return WebIDL::VersionError::create(realm, "Database version is greater than the requested version"_string);
    }

    // 8. Let connection be a new connection to db.
    auto connection = IDBDatabase::create(realm, *db);

    // 9. Set connection’s version to version.
    connection->set_version(version);

    // 10. If db’s version is less than version, then:
    if (db->version() < version) {
        // 1. Let openConnections be the set of all connections, except connection, associated with db.
        auto open_connections = db->associated_connections_except(connection);

        // 2. For each entry of openConnections that does not have its close pending flag set to true,
        //    queue a task to fire a version change event named versionchange at entry with db’s version and version.
        for (auto& entry : open_connections) {
            if (!entry->close_pending()) {
                HTML::queue_a_task(HTML::Task::Source::DatabaseAccess, nullptr, nullptr, JS::create_heap_function(realm.vm().heap(), [&realm, entry, db, version]() {
                    fire_a_version_change_event(realm, HTML::EventNames::versionchange, *entry, db->version(), version);
                }));
            }
        }

        // FIXME: 3. Wait for all of the events to be fired.

        // 4. If any of the connections in openConnections are still not closed,
        //    queue a task to fire a version change event named blocked at request with db’s version and version.
        for (auto& entry : open_connections) {
            if (entry->state() != IDBDatabase::ConnectionState::Closed) {
                HTML::queue_a_task(HTML::Task::Source::DatabaseAccess, nullptr, nullptr, JS::create_heap_function(realm.vm().heap(), [&realm, entry, db, version]() {
                    fire_a_version_change_event(realm, HTML::EventNames::blocked, *entry, db->version(), version);
                }));
            }
        }

        // 5. Wait until all connections in openConnections are closed.
        HTML::main_thread_event_loop().spin_until(JS::create_heap_function(realm.vm().heap(), [open_connections]() {
            for (auto const& entry : open_connections) {
                if (entry->state() != IDBDatabase::ConnectionState::Closed) {
                    return false;
                }
            }

            return true;
        }));

        // 6. Run upgrade a database using connection, version and request.
        upgrade_a_database(realm, connection, version, request);

        // 7. If connection was closed, return a newly created "AbortError" DOMException and abort these steps.
        if (connection->state() == IDBDatabase::ConnectionState::Closed) {
            return WebIDL::AbortError::create(realm, "Connection was closed"_string);
        }

        // FIXME: 8. If the upgrade transaction was aborted, run the steps to close a database connection with connection,
        // return a newly created "AbortError" DOMException and abort these steps.
    }

    // 11. Return connection.
    return connection;
}

bool fire_a_version_change_event(JS::Realm& realm, FlyString const& event_name, JS::NonnullGCPtr<DOM::EventTarget> target, u64 old_version, Optional<u64> new_version)
{
    IDBVersionChangeEventInit event_init = {};
    // 4. Set event’s oldVersion attribute to oldVersion.
    event_init.old_version = old_version;
    // 5. Set event’s newVersion attribute to newVersion.
    event_init.new_version = new_version;

    // 1. Let event be the result of creating an event using IDBVersionChangeEvent.
    // 2. Set event’s type attribute to e.
    auto event = IDBVersionChangeEvent::create(realm, event_name, event_init);

    // 3. Set event’s bubbles and cancelable attributes to false.
    event->set_bubbles(false);
    event->set_cancelable(false);

    // 6. Let legacyOutputDidListenersThrowFlag be false.
    auto legacy_output_did_listeners_throw_flag = false;

    // 7. Dispatch event at target with legacyOutputDidListenersThrowFlag.
    DOM::EventDispatcher::dispatch(target, *event, legacy_output_did_listeners_throw_flag);

    // 8. Return legacyOutputDidListenersThrowFlag.
    return legacy_output_did_listeners_throw_flag;
}

void close_a_database_connection(IDBDatabase& connection, bool forced)
{
    // 1. Set connection’s close pending flag to true.
    connection.set_close_pending(true);

    // FIXME: 2. If the forced flag is true, then for each transaction created using connection run abort a transaction with transaction and newly created "AbortError" DOMException.
    // FIXME: 3. Wait for all transactions created using connection to complete. Once they are complete, connection is closed.
    connection.set_state(IDBDatabase::ConnectionState::Closed);

    // 4. If the forced flag is true, then fire an event named close at connection.
    if (forced)
        connection.dispatch_event(DOM::Event::create(connection.realm(), HTML::EventNames::close));
}

void upgrade_a_database(JS::Realm& realm, JS::NonnullGCPtr<IDBDatabase> connection, u64 version, JS::NonnullGCPtr<IDBRequest> request)
{
    // 1. Let db be connection’s database.
    auto db = connection->associated_database();

    // 2. Let transaction be a new upgrade transaction with connection used as connection.
    auto transaction = IDBTransaction::create(realm, connection);

    // FIXME: 3. Set transaction’s scope to connection’s object store set.

    // 4. Set db’s upgrade transaction to transaction.
    db->set_upgrade_transaction(transaction);

    // 5. Set transaction’s state to inactive.
    transaction->set_state(IDBTransaction::TransactionState::Inactive);

    // FIXME: 6. Start transaction.

    // 7. Let old version be db’s version.
    auto old_version = db->version();

    // 8. Set db’s version to version. This change is considered part of the transaction, and so if the transaction is aborted, this change is reverted.
    db->set_version(version);

    // 9. Set request’s processed flag to true.
    request->set_processed(true);

    // 10. Queue a task to run these steps:
    HTML::queue_a_task(HTML::Task::Source::DatabaseAccess, nullptr, nullptr, JS::create_heap_function(realm.vm().heap(), [&realm, request, connection, transaction, old_version, version]() {
        // 1. Set request’s result to connection.
        request->set_result(connection);

        // 2. Set request’s transaction to transaction.
        request->set_transaction(transaction);

        // 3. Set request’s done flag to true.
        request->set_done(true);

        // 4. Set transaction’s state to active.
        transaction->set_state(IDBTransaction::TransactionState::Active);

        // 5. Let didThrow be the result of firing a version change event named upgradeneeded at request with old version and version.
        [[maybe_unused]] auto did_throw = fire_a_version_change_event(realm, HTML::EventNames::upgradeneeded, request, old_version, version);

        // 6. Set transaction’s state to inactive.
        transaction->set_state(IDBTransaction::TransactionState::Inactive);

        // FIXME: 7. If didThrow is true, run abort a transaction with transaction and a newly created "AbortError" DOMException.
    }));

    // FIXME: 11. Wait for transaction to finish.
}

}
