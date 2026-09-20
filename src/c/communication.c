#include <stddef.h> // offsetof
#include "communication.h"
#include "globals.h"
#include "weather.h"
#include "api.h"
#include "outbox_queue.h"

// Defines how an incoming message key maps to a specific field in the settings struct.
typedef struct {
  const uint32_t *message_key;
  size_t offset;
  uint8_t size; // numeric fields: 1/2/4 bytes; string fields: buffer capacity
  bool is_string;
} ClaySettingField;

// Helper macro for when the AppMessage key and the struct field share the exact same name.
#define SF(field) \
  { &MESSAGE_KEY_##field, offsetof(ClaySettings, field), sizeof(((ClaySettings *)0)->field), false }

// Helper macro for when the AppMessage key differs from the struct field name.
#define SF2(msg_key, field) \
  { &MESSAGE_KEY_##msg_key, offsetof(ClaySettings, field), sizeof(((ClaySettings *)0)->field), false }

// Helper macro for char[] settings fields (copied via snprintf, not a fixed-width numeric write).
#define SF_STR(field) \
  { &MESSAGE_KEY_##field, offsetof(ClaySettings, field), sizeof(((ClaySettings *)0)->field), true }

// Master lookup table for all configuration settings.
static const ClaySettingField CLAY_SETTINGS_FIELDS[] = {
  SF_STR(api_key),
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
  #if defined(PBL_HEALTH)
  SF(healthQuiet),
  SF(stepsBool),
  SF(hrBool),
  SF(stepsGoalBool),
  SF(stepsGoal),
  #endif
  SF2(hardcodeRivalBool, hardcodeRival), // messageKey/field names diverge
  SF(animationDelay),
  SF(countdownBool),
  SF(countdownTime),
  SF_STR(countdownCustomDate),
  SF_STR(countdownCustomTime),
  SF(countdownDisplay),
  SF(api),
  SF(api_quiet),
  SF(scoreDisplayBool),
  // SF(scoreUpdate), // no longer used - see CFBD_LIGHT_SYNC_INTERVAL_SECONDS in api.c
  SF(scoreLocation),
  SF(opponentSelect),
  SF(customOpponent),
  #ifndef PBL_PLATFORM_APLITE
  SF(weatherBool),
  SF(weatherQuiet),
  SF(weatherUnits),
  SF(bagBool),
  SF(rankingBool),
  SF(winBool),
  SF(confBool),
  SF(bowlBool),
  SF(champBool),
  #endif
  SF(watchUpdate),
};

// Clean up macros so they don't pollute the rest of the namespace
#undef SF
#undef SF2
#undef SF_STR

#define CLAY_SETTINGS_FIELDS_COUNT (sizeof(CLAY_SETTINGS_FIELDS) / sizeof(CLAY_SETTINGS_FIELDS[0]))

// Helper function to find a setting's mapping definition based on an incoming AppMessage key.
static const ClaySettingField *prv_find_field(uint32_t key) {
  for (uint32_t i = 0; i < CLAY_SETTINGS_FIELDS_COUNT; i++) {
    if (key == *CLAY_SETTINGS_FIELDS[i].message_key) return &CLAY_SETTINGS_FIELDS[i];
  }
  return NULL;
}

// Iterates through incoming dictionary elements and updates the global settings
void configuration_callback(DictionaryIterator *iterator, void *context) {
  bool settings_changed = false;
  uint8_t previous_favorite_team = settings.FavoriteTeam;

  for (Tuple *t = dict_read_first(iterator); t != NULL; t = dict_read_next(iterator)) {
    // Look up which settings field this key maps to
    const ClaySettingField *field = prv_find_field(t->key);
    if (!field) continue;

    settings_changed = true;
    uint8_t *field_ptr = (uint8_t *)&settings + field->offset;

    if (field->is_string) {
      snprintf((char *)field_ptr, field->size, "%s", t->value->cstring);
      continue;
    }

    int32_t value = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring) : t->value->int32;

    // Write directly into the settings struct at the field's offset
    switch (field->size) {
      case 1: *(uint8_t *)field_ptr = (uint8_t)value; break;
      case 2: *(uint16_t *)field_ptr = (uint16_t)value; break;
      case 4: *(uint32_t *)field_ptr = (uint32_t)value; break;
      default:
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_ERROR, "Clay setting field size %d unsupported", field->size);
      #endif
      break;
    }
  }

  // Save and apply if any settings were changed
  if (settings_changed) {
    globals_prv_save_settings();
    if (settings.FavoriteTeam != previous_favorite_team) {
      globals_prv_load_team_data();
    }
    globals_prv_update_display();
  }
}

// AppMessage received handler
void inbox_received_callback(DictionaryIterator *iterator, void *context) {

  
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Received!");
  APP_LOG(APP_LOG_LEVEL_INFO, "Configuration - Dict size: %d", dict_size(iterator));
  #endif
  configuration_callback(iterator, context);
  api_cfbd_callback(iterator, context);  
  #ifndef PBL_PLATFORM_APLITE
  weather_callback(iterator, context);
  #endif
}

// Triggered if the phone tries to send data to the watch, but the watch fails to receive it.
void inbox_dropped_callback(AppMessageResult reason, void *context) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  #endif
}

// Triggered if the watch attempts to send a message to the phone, but fails.
void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", reason);
  #endif
  outbox_queue_on_result();
}

// Triggered when a message is successfully delivered from the watch to the phone.
void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  #endif
  outbox_queue_on_result();
}