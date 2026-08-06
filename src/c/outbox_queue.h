// src/c/outbox_queue.h
#pragma once
#include <pebble.h>

/**
 * Pebble's AppMessage outbox only allows one message in flight at a time -
 * app_message_outbox_begin() fails with APP_MSG_BUSY if called again
 * before the previous message's outbox_sent/outbox_failed callback has
 * fired. Several modules (weather requests in timekeeping.c, the various
 * CFBD sync requests in api.c) can each want to send at any time, and
 * during an ~8s CFBD light sync (many small team-by-team round trips) the
 * odds of two of these landing close together are high. Most of the
 * existing call sites didn't check app_message_outbox_begin()'s return
 * value, so a collision meant a silently dropped or malformed send.
 *
 * This module serializes all of that: callers hand over a builder
 * function instead of calling app_message_outbox_begin/send directly, and
 * only one item is ever actually in flight - the rest wait in a small
 * FIFO queue and get sent one at a time as prior sends complete.
 */

// Fills in the outgoing dictionary for one queued send. Called with a
// fresh, valid iterator from app_message_outbox_begin() - just write your
// keys/values into it, don't call outbox_send() yourself.
typedef void (*OutboxBuilderFn)(DictionaryIterator *iter);

// Queues a send. Returns false (and logs) if the queue is full, though
// under normal use it should never fill given how infrequently these
// fire relative to how fast each completes.
bool outbox_queue_send(OutboxBuilderFn builder);

// Call from communication.c's outbox_sent_callback and
// outbox_failed_callback - either way, the outbox is free again, so this
// starts the next queued send if there is one.
void outbox_queue_on_result(void);
