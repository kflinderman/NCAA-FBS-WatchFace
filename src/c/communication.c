#include <stddef.h> // offsetof
#include "communication.h"
#include "globals.h"
#include "weather.h"
#include "api.h"
#include "outbox_queue.h"

// One entry per Clay-driven ClaySettings field: which AppMessage key feeds
// it, and where/how big it is in the struct. offsetof/sizeof are computed
// by the compiler, so adding, removing, or reordering a setting is a single
// line here - there's no positional index or count to keep in sync by hand
// (the previous switch(idx) on array position was one edit away from
// silently writing a value into the wrong field, which is exactly what
// happened when opponentBool needed to be removed here).
//
// message_key is a POINTER to the generated MESSAGE_KEY_* symbol, not the
// key value itself: MESSAGE_KEY_* are extern const uint32_t (linked, not
// #define'd), so their value isn't known until link time and can't sit
// directly in a static initializer - only their address can (a valid
// link-time constant). Dereference at lookup time in prv_find_field().
typedef struct {
  const uint32_t *message_key;
  size_t offset;
  uint8_t size; // bytes: 1 (uint8_t/bool), 2 (uint16_t), or 4 (uint32_t)
} ClaySettingField;

#define SF(field) \
  { &MESSAGE_KEY_##field, offsetof(ClaySettings, field), sizeof(((ClaySettings *)0)->field) }
#define SF2(msg_key, field) \
  { &MESSAGE_KEY_##msg_key, offsetof(ClaySettings, field), sizeof(((ClaySettings *)0)->field) }

// Stored in Flash/ROM — 0 bytes of RAM usage
static const ClaySettingField CLAY_SETTINGS_FIELDS[] = {
  SF(DisconnectVibration),
  SF(ReconnectVibration),
  SF(LowBatteryPercent),
  SF(LowBatteryVibration),
  SF(EmptyBatteryPercent),
  SF(EmptyBatteryVibration),
  SF(DisplayTeam),
  SF(FavoriteTeam),
  SF(BeatTeam),
  SF(animationSensitivity),
  SF(quietTimeBool),
  SF(quietTimeStart),
  SF(quietTimeEnd),
  SF(animationsBatt),
  SF(animationsCustom),
  SF(healthQuiet),
  SF(stepsBool),
  SF(hrBool),
  SF(stepsGoalBool),
  SF(stepsGoal),
  SF2(hardcodeRivalBool, hardcodeRival), // messageKey/field names diverge
  SF(bagBool),
  SF(animationDelay),
  SF(countdownBool),
  SF(countdownTime),
  SF(countdownCustomDate),
  SF(countdownCustomTime),
  SF(countdownDisplay),
  SF(api),
  SF(api_quiet),
  SF(scoreDisplayBool),
  SF(scoreUpdate),
  SF(scoreLocation),
  SF(opponentSelect),
  SF(customOpponent),
  SF(weatherBool),
  SF(weatherQuiet),
  SF(weatherUnits),
  SF(rankingBool),
  SF(winBool),
  SF(confBool),
  SF(bowlBool),
  SF(champBool),
};

#undef SF
#undef SF2

#define CLAY_SETTINGS_FIELDS_COUNT (sizeof(CLAY_SETTINGS_FIELDS) / sizeof(CLAY_SETTINGS_FIELDS[0]))

static const ClaySettingField *prv_find_field(uint32_t key) {
  for (uint32_t i = 0; i < CLAY_SETTINGS_FIELDS_COUNT; i++) {
    if (key == *CLAY_SETTINGS_FIELDS[i].message_key) return &CLAY_SETTINGS_FIELDS[i];
  }
  return NULL;
}

void configuration_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Configuration - Dict size: %d", dict_size(iterator));
  bool settings_changed = false;

  for (Tuple *t = dict_read_first(iterator); t != NULL; t = dict_read_next(iterator)) {
    // 1. API key string assignment
    if (t->key == MESSAGE_KEY_api_key) {
      snprintf(settings.api_key, sizeof(settings.api_key), "%s", t->value->cstring);
      settings_changed = true;
      continue;
    }

    // 2. Look up which settings field this key maps to
    const ClaySettingField *field = prv_find_field(t->key);
    if (!field) continue;

    int32_t value = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring) : t->value->int32;
    settings_changed = true;

    // 3. Write directly into the settings struct at the field's offset
    uint8_t *field_ptr = (uint8_t *)&settings + field->offset;
    switch (field->size) {
      case 1: *(uint8_t *)field_ptr = (uint8_t)value; break;
      case 2: *(uint16_t *)field_ptr = (uint16_t)value; break;
      case 4: *(uint32_t *)field_ptr = (uint32_t)value; break;
      default:
        APP_LOG(APP_LOG_LEVEL_ERROR, "Clay setting field size %d unsupported", field->size);
        break;
    }
  }

  // Save and apply if any settings were changed
  if (settings_changed) {
    globals_prv_save_settings();
    globals_prv_update_display();
  }
}

// AppMessage received handler
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Received!");
  APP_LOG(APP_LOG_LEVEL_INFO, "Configuration - Dict size: %d", dict_size(iterator));
  configuration_callback(iterator, context);
  weather_callback(iterator, context);
  api_cfbd_callback(iterator, context);
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", reason);
  outbox_queue_on_result();
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  outbox_queue_on_result();
}