#pragma once
#include <pebble.h>

typedef void (*OutboxBuilderFn)(DictionaryIterator *iter);
bool outbox_queue_send(OutboxBuilderFn builder);
void outbox_queue_on_result(void);
