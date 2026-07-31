#include "communication.h"
#include "globals.h"
#include "weather.h"
#include "api.h"
#include "outbox_queue.h"

void configuration_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Configuration - Dict size: %d", dict_size(iterator));
  // Check for Clay settings data
  uint32_t claysettings_id[] = {
    MESSAGE_KEY_DisconnectVibration,
    MESSAGE_KEY_ReconnectVibration,
    MESSAGE_KEY_LowBatteryPercent,
    MESSAGE_KEY_LowBatteryVibration,
    MESSAGE_KEY_EmptyBatteryPercent,
    MESSAGE_KEY_EmptyBatteryVibration,
    MESSAGE_KEY_DisplayTeam,
    MESSAGE_KEY_FavoriteTeam,
    MESSAGE_KEY_BeatTeam,
    MESSAGE_KEY_animationSensitivity,
    MESSAGE_KEY_quietTimeBool,
    MESSAGE_KEY_quietTimeStart,
    MESSAGE_KEY_quietTimeEnd,
    MESSAGE_KEY_animationsBatt,
    MESSAGE_KEY_animationsCustom,
    MESSAGE_KEY_stepsBool,
    MESSAGE_KEY_hrBool,
    MESSAGE_KEY_stepsGoalBool,
    MESSAGE_KEY_stepsGoal,
    MESSAGE_KEY_hardcodeRivalBool, 
    MESSAGE_KEY_donate,
    MESSAGE_KEY_bagBool,
    MESSAGE_KEY_animationDelay,
    MESSAGE_KEY_countdownBool,
    MESSAGE_KEY_countdownTime,
    MESSAGE_KEY_countdownCustomDate,
    MESSAGE_KEY_countdownCustomTime,
    MESSAGE_KEY_countdownDisplay,
    MESSAGE_KEY_api,
    MESSAGE_KEY_api_quiet,
    MESSAGE_KEY_scoreDisplayBool,
    MESSAGE_KEY_scoreUpdate,
    MESSAGE_KEY_scoreLocation,
    MESSAGE_KEY_opponentBool,
    MESSAGE_KEY_opponentSelect,
    MESSAGE_KEY_customOpponent,
    MESSAGE_KEY_weatherBool,
    MESSAGE_KEY_weatherQuiet,
    MESSAGE_KEY_weatherUnits,
    MESSAGE_KEY_rankingBool,
    MESSAGE_KEY_winBool,
    MESSAGE_KEY_confBool,
    MESSAGE_KEY_bowlBool,
    MESSAGE_KEY_champBool,
  };

  bool settings_changed = false;
  
  Tuple *key_tuple = dict_find(iterator, MESSAGE_KEY_api_key);
  if (key_tuple){
    snprintf(settings.api_key, sizeof(settings.api_key), "%s", key_tuple->value->cstring);
    settings_changed = true;
  }
  

  for (uint16_t x = 0; x < 44; x++) {
    Tuple *temp_t = dict_find(iterator, claysettings_id[x]);
    
    if (temp_t) {
      int32_t value = 0;

      if (temp_t->type == TUPLE_CSTRING) {
        // Manual, safer string-to-int conversion instead of strtol
        const char *str = temp_t->value->cstring;
        value = 0;
        for (int i = 0; str[i] != '\0'; i++) {
          if (str[i] >= '0' && str[i] <= '9') {
            value = value * 10 + (str[i] - '0');
          }
        }
      } else if (temp_t->type == TUPLE_INT) {
        value = temp_t->value->int32;
      }

      // Directly assign to settings struct
      switch (x) {
        case 0: settings.DisconnectVibration = value; break;
        case 1: settings.ReconnectVibration = value; break;
        case 2: settings.LowBatteryPercent = value; break;
        case 3: settings.LowBatteryVibration = value; break;
        case 4: settings.EmptyBatteryPercent = value; break;
        case 5: settings.EmptyBatteryVibration = value; break;
        case 6: settings.DisplayTeam = value; break;
        case 7: settings.FavoriteTeam = value; break;
        case 8: settings.BeatTeam = value; break;
        case 9: settings.animationSensitivity = value; break;
        case 10: settings.quietTimeBool = value; break;
        case 11: settings.quietTimeStart = value; break;
        case 12: settings.quietTimeEnd = value; break;
        case 13: settings.animationsBatt = value; break;
        case 14: settings.animationsCustom = value; break;
        case 15: settings.stepsBool = value; break;
        case 16: settings.hrBool = value; break;
        case 17: settings.stepsGoalBool = value; break;
        case 18: settings.stepsGoal = value; break;
        case 19: settings.hardcodeRival = value; break;
        case 20: settings.donate = value; break;
        case 21: settings.bagBool = value; break;
        case 22: settings.animationDelay = value; break;
        case 23: settings.countdownBool = value; break;
        case 24: settings.countdownTime = value; break;
        case 25: settings.countdownCustomDate = value; break;
        case 26: settings.countdownCustomTime = value; break;
        case 27: settings.countdownDisplay = value; break;
        case 28: settings.api = value; break;
        case 29: settings.api_quiet = value; break;
        case 30: settings.scoreDisplayBool = value; break;
        case 31: settings.scoreUpdate = value; break;
        case 32: settings.scoreLocation = value; break;
        case 33: settings.opponentBool = value; break;
        case 34: settings.opponentSelect = value; break;
        case 35: settings.customOpponent = value; break;
        case 36: settings.weatherBool = value; break;
        case 37: settings.weatherQuiet = value; break;
        case 38: settings.weatherUnits = value; break;
        case 39: settings.rankingBool = value; break;
        case 40: settings.winBool = value; break;
        case 41: settings.confBool = value; break;
        case 42: settings.bowlBool = value; break;
        case 43: settings.champBool = value; break;
      }

      settings_changed = true;
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
  configuration_callback(iterator, context);
  weather_callback(iterator, context);
  api_cfbd_callback(iterator, context);
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  outbox_queue_on_result();
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  outbox_queue_on_result();
}
