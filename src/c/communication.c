#include "communication.h"
#include "globals.h"


void configuration_callback(DictionaryIterator *iterator, void *context){
  APP_LOG(APP_LOG_LEVEL_INFO, "Configuration");
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
    MESSAGE_KEY_countdownCustom,
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
    MESSAGE_KEY_weatherManual,
    MESSAGE_KEY_weatherLocation,
  };

  bool settings_changed = false;
  
  Tuple *key_tuple = dict_find(iterator, MESSAGE_KEY_api_key);
  if (key_tuple){
    snprintf(settings.api_key, sizeof(settings.api_key), "%s", key_tuple->value->cstring);
    settings_changed = true;
  }
  

  for (uint16_t x = 0; x < 40; x++) {
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
        case 25: settings.countdownCustom = value; break;
        case 26: settings.countdownDisplay = value; break;
        case 27: settings.api = value; break;
        case 28: settings.api_quiet = value; break;
        case 29: settings.scoreDisplayBool = value; break;
        case 30: settings.scoreUpdate = value; break;
        case 31: settings.scoreLocation = value; break;
        case 32: settings.opponentBool = value; break;
        case 33: settings.opponentSelect = value; break;
        case 34: settings.customOpponent = value; break;
        case 35: settings.weatherBool = value; break;
        case 36: settings.weatherQuiet = value; break;
        case 37: settings.weatherUnits = value; break;
        case 38: settings.weatherManual = value; break;
        case 39: settings.weatherLocation = value; break;
      }

      settings_changed = true;
    }
  }

  // Save and apply if any settings were changed
  if (settings_changed) {
    prv_save_settings();
    prv_update_display();
  }
}

void weather_callback(DictionaryIterator *iterator, void *context){
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  //Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  if (temp_tuple) {
    static char temperature_buffer[8];
    APP_LOG(APP_LOG_LEVEL_INFO, "Temperature: %d", (int)temp_tuple->value->int32);
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)temp_tuple->value->int32);
    text_layer_set_text(s_weather_layer, temperature_buffer);
  }
}

// AppMessage received handler
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Message Received!");
  configuration_callback(iterator, context);
  weather_callback(iterator, context);
}

void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
