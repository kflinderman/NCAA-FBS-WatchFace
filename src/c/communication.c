#include "communication.h"
#include "globals.h"

// AppMessage received handler
void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  /*
  // Check for weather data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  if (temp_tuple && conditions_tuple) {
    static char temperature_buffer[8];
    static char conditions_buffer[32];
    static char weather_layer_buffer[32];

    int temp_value = (int)temp_tuple->value->int32;

    // Convert to Fahrenheit if setting is enabled
    if (settings.TemperatureUnit) {
      temp_value = (temp_value * 9 / 5) + 32;
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°F", temp_value);
    } else {
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", temp_value);
    }

    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s %s", temperature_buffer, conditions_buffer);
  }
  */

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
  };

  bool settings_changed = false;

  for (uint16_t x = 0; x < 22; x++) {
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
      }

      settings_changed = true;
    }
  }

  // Save and apply if any settings were changed
  if (settings_changed) {
    prv_save_settings();
    prv_update_display();

    // Refetch weather if the temperature unit changed so the display updates
    /*
    if (temp_unit_t) {
      DictionaryIterator *iter;
      app_message_outbox_begin(&iter);
      dict_write_uint8(iter, MESSAGE_KEY_REQUEST_WEATHER, 1);
      app_message_outbox_send();
    }
    */
  }
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
