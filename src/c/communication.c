#include "communication.h"
#include "globals.h"
#include "weather.h"
#include "api.h"
#include "outbox_queue.h"

// Stored in Flash/ROM — 0 bytes of RAM usage
static const uint32_t * const CLAY_KEYS[] = {
  &MESSAGE_KEY_DisconnectVibration,   // 0
  &MESSAGE_KEY_ReconnectVibration,    // 1
  &MESSAGE_KEY_LowBatteryPercent,     // 2
  &MESSAGE_KEY_LowBatteryVibration,   // 3
  &MESSAGE_KEY_EmptyBatteryPercent,   // 4
  &MESSAGE_KEY_EmptyBatteryVibration, // 5
  &MESSAGE_KEY_DisplayTeam,           // 6
  &MESSAGE_KEY_FavoriteTeam,          // 7
  &MESSAGE_KEY_BeatTeam,              // 8
  &MESSAGE_KEY_animationSensitivity,  // 9
  &MESSAGE_KEY_quietTimeBool,         // 10
  &MESSAGE_KEY_quietTimeStart,        // 11
  &MESSAGE_KEY_quietTimeEnd,          // 12
  &MESSAGE_KEY_animationsBatt,        // 13
  &MESSAGE_KEY_animationsCustom,      // 14
  &MESSAGE_KEY_healthQuiet,           // 15
  &MESSAGE_KEY_stepsBool,             // 16
  &MESSAGE_KEY_hrBool,                // 17
  &MESSAGE_KEY_stepsGoalBool,         // 18
  &MESSAGE_KEY_stepsGoal,             // 19
  &MESSAGE_KEY_hardcodeRivalBool,     // 20
  &MESSAGE_KEY_donate,                // 21
  &MESSAGE_KEY_bagBool,               // 22
  &MESSAGE_KEY_animationDelay,        // 23
  &MESSAGE_KEY_countdownBool,         // 24
  &MESSAGE_KEY_countdownTime,         // 25
  &MESSAGE_KEY_countdownCustomDate,   // 26
  &MESSAGE_KEY_countdownCustomTime,   // 27
  &MESSAGE_KEY_countdownDisplay,      // 28
  &MESSAGE_KEY_api,                   // 29
  &MESSAGE_KEY_api_quiet,             // 30
  &MESSAGE_KEY_scoreDisplayBool,      // 31
  &MESSAGE_KEY_scoreUpdate,           // 32
  &MESSAGE_KEY_scoreLocation,         // 33
  &MESSAGE_KEY_opponentBool,          // 34
  &MESSAGE_KEY_opponentSelect,        // 35
  &MESSAGE_KEY_customOpponent,        // 36
  &MESSAGE_KEY_weatherBool,           // 37
  &MESSAGE_KEY_weatherQuiet,          // 38
  &MESSAGE_KEY_weatherUnits,          // 39
  &MESSAGE_KEY_rankingBool,           // 40
  &MESSAGE_KEY_winBool,               // 41
  &MESSAGE_KEY_confBool,              // 42
  &MESSAGE_KEY_bowlBool,              // 43
  &MESSAGE_KEY_champBool              // 44
};

#define CLAY_KEYS_COUNT (sizeof(CLAY_KEYS) / sizeof(CLAY_KEYS[0]))

static int prv_get_key_index(uint32_t key) {
  for (uint32_t i = 0; i < CLAY_KEYS_COUNT; i++) {
    if (key == *CLAY_KEYS[i]) return (int)i;
  }
  return -1;
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

    // 2. Map dynamic key pointer to an integer index (0-44)
    int idx = prv_get_key_index(t->key);
    if (idx < 0) continue;

    int32_t value = (t->type == TUPLE_CSTRING) ? atoi(t->value->cstring) : t->value->int32;
    settings_changed = true;

    // 3. Switch using valid integer constant expressions
    switch (idx) {
      case 0:  settings.DisconnectVibration = value; break;
      case 1:  settings.ReconnectVibration = value; break;
      case 2:  settings.LowBatteryPercent = value; break;
      case 3:  settings.LowBatteryVibration = value; break;
      case 4:  settings.EmptyBatteryPercent = value; break;
      case 5:  settings.EmptyBatteryVibration = value; break;
      case 6:  settings.DisplayTeam = value; break;
      case 7:  settings.FavoriteTeam = value; break;
      case 8:  settings.BeatTeam = value; break;
      case 9:  settings.animationSensitivity = value; break;
      case 10: settings.quietTimeBool = value; break;
      case 11: settings.quietTimeStart = value; break;
      case 12: settings.quietTimeEnd = value; break;
      case 13: settings.animationsBatt = value; break;
      case 14: settings.animationsCustom = value; break;
      case 15: settings.healthQuiet = value; break;
      case 16: settings.stepsBool = value; break;
      case 17: settings.hrBool = value; break;
      case 18: settings.stepsGoalBool = value; break;
      case 19: settings.stepsGoal = value; break;
      case 20: settings.hardcodeRival = value; break;
      case 21: settings.donate = value; break;
      case 22: settings.bagBool = value; break;
      case 23: settings.animationDelay = value; break;
      case 24: settings.countdownBool = value; break;
      case 25: settings.countdownTime = value; break;
      case 26: settings.countdownCustomDate = value; break;
      case 27: settings.countdownCustomTime = value; break;
      case 28: settings.countdownDisplay = value; break;
      case 29: settings.api = value; break;
      case 30: settings.api_quiet = value; break;
      case 31: settings.scoreDisplayBool = value; break;
      case 32: settings.scoreUpdate = value; break;
      case 33: settings.scoreLocation = value; break;
      case 34: settings.opponentBool = value; break;
      case 35: settings.opponentSelect = value; break;
      case 36: settings.customOpponent = value; break;
      case 37: settings.weatherBool = value; break;
      case 38: settings.weatherQuiet = value; break;
      case 39: settings.weatherUnits = value; break;
      case 40: settings.rankingBool = value; break;
      case 41: settings.winBool = value; break;
      case 42: settings.confBool = value; break;
      case 43: settings.bowlBool = value; break;
      case 44: settings.champBool = value; break;
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