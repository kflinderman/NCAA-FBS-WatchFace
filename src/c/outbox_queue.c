// src/c/outbox_queue.c
#include "outbox_queue.h"

#define OUTBOX_QUEUE_MAX 8

static OutboxBuilderFn outbox_queue[OUTBOX_QUEUE_MAX];
static int outbox_queue_head = 0;
static int outbox_queue_tail = 0;
static int outbox_queue_count = 0;
static bool outbox_busy = false;

static void outbox_queue_try_send(void) {
  if (outbox_busy || outbox_queue_count == 0) {
    return;
  }

  OutboxBuilderFn builder = outbox_queue[outbox_queue_head];
  outbox_queue_head = (outbox_queue_head + 1) % OUTBOX_QUEUE_MAX;
  outbox_queue_count--;

  DictionaryIterator *iter;
  AppMessageResult begin_result = app_message_outbox_begin(&iter);
  if (begin_result != APP_MSG_OK) {
    // Outbox genuinely busy (shouldn't happen given outbox_busy already
    // gates this, but AppMessage state can be affected by other things
    // like a Bluetooth disconnect) - drop this one item and try the next
    // rather than getting stuck.
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue: begin failed (%d) - dropping this item", begin_result);
    outbox_queue_try_send();
    return;
  }

  builder(iter);

  outbox_busy = true;
  AppMessageResult send_result = app_message_outbox_send();
  if (send_result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue: send failed (%d) - trying next item", send_result);
    outbox_busy = false;
    outbox_queue_try_send();
  }
  // On success, outbox_busy stays true until outbox_queue_on_result() is
  // called from outbox_sent_callback/outbox_failed_callback.
}

bool outbox_queue_send(OutboxBuilderFn builder) {
  if (outbox_queue_count >= OUTBOX_QUEUE_MAX) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue full - dropping send");
    return false;
  }

  outbox_queue[outbox_queue_tail] = builder;
  outbox_queue_tail = (outbox_queue_tail + 1) % OUTBOX_QUEUE_MAX;
  outbox_queue_count++;

  outbox_queue_try_send();
  return true;
}

void outbox_queue_on_result(void) {
  outbox_busy = false;
  outbox_queue_try_send();
}
