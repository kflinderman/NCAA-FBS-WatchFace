#include "outbox_queue.h"

// Maximum number of pending messages the queue can hold
#define OUTBOX_QUEUE_MAX 8

static OutboxBuilderFn outbox_queue[OUTBOX_QUEUE_MAX];
static int outbox_queue_head = 0;
static int outbox_queue_tail = 0;
static int outbox_queue_count = 0;

// Lock flag to prevent sending if a message is currently in transit.
static bool outbox_busy = false;

// Internal worker function to process the next item in the queue.
static void outbox_queue_try_send() {
  if (outbox_busy || outbox_queue_count == 0) {
    return;
  }

  OutboxBuilderFn builder = outbox_queue[outbox_queue_head];
  outbox_queue_head = (outbox_queue_head + 1) % OUTBOX_QUEUE_MAX;
  outbox_queue_count--;

  // Prepare the Pebble outbox for a new message
  DictionaryIterator *iter;
  AppMessageResult begin_result = app_message_outbox_begin(&iter);
  if (begin_result != APP_MSG_OK) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue: begin failed (%d) - dropping this item", begin_result);
    #endif
    outbox_queue_try_send();
    return;
  }

  // Execute the function pointer. This is where the actual data gets written.
  builder(iter);

  //Send the message
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

// Gunction to add a new message builder to the queue
bool outbox_queue_send(OutboxBuilderFn builder) {
  // Check if the buffer is full
  if (outbox_queue_count >= OUTBOX_QUEUE_MAX) {
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox queue full - dropping send");
    #endif
    return false;
  }

  // Insert the new builder function at the back of the queue
  outbox_queue[outbox_queue_tail] = builder;
  outbox_queue_tail = (outbox_queue_tail + 1) % OUTBOX_QUEUE_MAX;
  outbox_queue_count++;

  // Attempt to flush the queue (will only fire if outbox_busy is false)
  outbox_queue_try_send();
  return true;
}

// Public function to notify the queue that a message finished (success or failure).
// This MUST be called inside your app's `outbox_sent_callback` and `outbox_failed_callback`.
void outbox_queue_on_result() {
  outbox_busy = false;     // Unlock the queue
  outbox_queue_try_send(); // Immediately try to send the next item
}
