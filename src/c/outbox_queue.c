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
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue: begin failed (%d) - dropping this item", begin_result);
    #endif
    outbox_queue_try_send();
    return;
  }

  builder(iter);

  outbox_busy = true;
  AppMessageResult send_result = app_message_outbox_send();
  if (send_result != APP_MSG_OK) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue: send failed (%d) - trying next item", send_result);
    #endif
    outbox_busy = false;
    outbox_queue_try_send();
  }
}

bool outbox_queue_send(OutboxBuilderFn builder) {
  if (outbox_queue_count >= OUTBOX_QUEUE_MAX) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue full - dropping send");
    #endif
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
