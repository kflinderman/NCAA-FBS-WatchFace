#include "globals.h"
#include "health.h"
#include "drawing.h"
#include "weather.h"
#include "display.h"
#include "timekeeping.h"
#include "animation.h"
#include "api.h"


/*******************************************
 * Definitions for all extern globals
 *******************************************/
ClaySettings settings;

Window *s_main_window;
TextLayer* s_text_layers[NUM_TEXT_LAYERS];

#if defined(PBL_HEALTH)
Layer *hr_icon, *step_ladder;
bool noHR = true;
#endif

GBitmap* s_gbitmap_layers[NUM_GBITMAP_LAYERS];
BitmapLayer* s_bitmap_layers[NUM_BITMAP_LAYERS];
Layer* s_layers[NUM_GENERIC_LAYERS];

//GFont s_gfont[NUM_GFONT];
GFont s_font, s_wIcon;

char s_time_text[6], s_countdown_text[6], s_score_text[6], s_home_text[5], s_away_text[5], s_day_text[5], s_hour_text[5];
int16_t s_prev_y = 0;
bool s_bt_connected = false;
bool s_animation = false;
bool s_favorite_team_data_missing = false;
bool after_time = true;
bool gametime = false;
BatteryChargeState s_battery_state;
bool s_bt_history = true;
int16_t s_batt_history = 0;
int32_t current_time_integer;
int16_t temperatureValue = 0;
int16_t conditionValue = 0;

uint8_t beat_spot, beat_primary;


void globals_what2show(const char *leftText, const char *rightText, const char *mainText, bool extras, bool Ishow){
  text_layer_set_text(s_text_layers[TEXT_LAYER_HOME], leftText);
  text_layer_set_text(s_text_layers[TEXT_LAYER_AWAY], rightText);
  text_layer_set_text(s_text_layers[TEXT_LAYER_TIME], mainText);
  
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_HOME]), extras);
  layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_AWAY]), extras);
  layer_set_hidden(s_layers[LAYER_SCORE_I], Ishow);
}

void globals_prv_default_settings() {
  settings.DisconnectVibration = 3;
  settings.ReconnectVibration = 1;
  settings.LowBatteryPercent = 30;
  settings.LowBatteryVibration = 1;
  settings.EmptyBatteryPercent = 10;
  settings.EmptyBatteryVibration = 2;
  settings.DisplayTeam = 0;
  #ifdef TESTING
  settings.FavoriteTeam = 1;
  settings.BeatTeam = 0;
  #else
  settings.FavoriteTeam = 108;
  settings.BeatTeam = 26;
  #endif
  settings.animationSensitivity = 1200;
  settings.quietTimeBool = false;
  settings.quietTimeStart = 2330;
  settings.quietTimeEnd = 630;
  settings.animationsBatt = 0;
  settings.animationsCustom = 30;
  #if defined(PBL_HEALTH)
  settings.healthQuiet = false;
  settings.stepsBool = false;
  settings.hrBool = false;
  settings.stepsGoalBool = false;
  settings.stepsGoal = 10000;
  #endif
  settings.hardcodeRival = 0;
  settings.animationDelay = false;
  settings.countdownBool = false;
  settings.countdownTime = 0;
  settings.countdownCustomDate = 0;
  settings.countdownCustomTime = 0;
  settings.countdownDisplay = 1;
  settings.api = false;
  settings.api_quiet = false;
  settings.scoreDisplayBool = false;
  settings.scoreUpdate = 5;
  settings.scoreLocation = 1;
  settings.opponentBool = false;
  settings.opponentSelect = 0;
  settings.customOpponent = 0;
  #ifndef PBL_PLATFORM_APLITE
  settings.weatherBool = false;
  settings.weatherQuiet = false;
  settings.weatherUnits = 0;
  settings.bagBool = false;
  settings.rankingBool = false;
  settings.winBool = false;
  settings.confBool = false;
  settings.bowlBool = false;
  settings.champBool = false;
  #endif
  settings.cfbd.next_season_first_game_ts = 0;
  settings.cfbd.current_season_year = 0;
  settings.cfbd.last_full_sync_ts = 0;
  settings.cfbd.last_light_sync_ts = 0;
  settings.cfbd.api_calls_this_month = 0;
  settings.cfbd.api_calls_monthly_limit = 0; // unknown until first full sync reports it
  settings.cfbd.api_data_valid = false;
  settings.watchUpdate = 1;
}

void globals_prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void globals_prv_load_settings() {
  globals_prv_default_settings();
  if (persist_exists(SETTINGS_KEY) && persist_get_size(SETTINGS_KEY) == sizeof(ClaySettings)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  }
  // Bounds-check team indices before they're used to index TEAMS[]
  #ifdef TESTING
  if (settings.FavoriteTeam >= TEAMS_COUNT) settings.FavoriteTeam = 1;
  if (settings.BeatTeam >= TEAMS_COUNT) settings.BeatTeam = 0;
  #else
  if (settings.FavoriteTeam >= TEAMS_COUNT) settings.FavoriteTeam = 108;
  if (settings.BeatTeam >= TEAMS_COUNT) settings.BeatTeam = 26;
  #endif

  globals_prv_load_team_data();
}

static void prv_team_cache_touch(PersistedTeamCache *cache, PersistedTeamData entry) {
  uint8_t found = MAX_CACHED_FAVORITE_TEAMS;
  for (uint8_t i = 0; i < MAX_CACHED_FAVORITE_TEAMS; i++) {
    if (cache->slots[i].team_index == entry.team_index) {
      found = i;
      break;
    }
  }

  uint8_t shift_end = (found < MAX_CACHED_FAVORITE_TEAMS) ? found : (MAX_CACHED_FAVORITE_TEAMS - 1);
  for (uint8_t i = shift_end; i > 0; i--) {
    cache->slots[i] = cache->slots[i - 1];
  }
  cache->slots[0] = entry;
}

void globals_prv_save_team_data(void) {
  if (settings.FavoriteTeam >= TEAMS_COUNT) return;

  PersistedTeamCache cache;
  if (persist_exists(TEAM_DATA_KEY) && persist_get_size(TEAM_DATA_KEY) == sizeof(PersistedTeamCache)) {
    persist_read_data(TEAM_DATA_KEY, &cache, sizeof(cache));
  } else {
    for (uint8_t i = 0; i < MAX_CACHED_FAVORITE_TEAMS; i++) {
      cache.slots[i].team_index = TEAM_CACHE_EMPTY_SLOT;
    }
  }

  Team *fav = &TEAMS[settings.FavoriteTeam];
  PersistedTeamData entry = {
    .team_index = settings.FavoriteTeam,
    .vs_id = fav->vs_id,
    .score = fav->score,
    .vs_score = fav->vs_score,
    #ifndef PBL_PLATFORM_APLITE
    .ranking = fav->ranking,
    .wins = fav->wins,
    .postseasonGames = fav->postseasonGames,
    .postseasonWins = fav->postseasonWins,
    .postseasonLosses = fav->postseasonLosses,
    #endif
    .completed = fav->completed,
    .gametime = fav->gametime,
  };

  prv_team_cache_touch(&cache, entry);
  persist_write_data(TEAM_DATA_KEY, &cache, sizeof(cache));
}

void globals_prv_load_team_data(void) {
  if (settings.FavoriteTeam >= TEAMS_COUNT) return;
  if (!persist_exists(TEAM_DATA_KEY) || persist_get_size(TEAM_DATA_KEY) != sizeof(PersistedTeamCache)) {
    s_favorite_team_data_missing = true;
    return;
  }

  PersistedTeamCache cache;
  persist_read_data(TEAM_DATA_KEY, &cache, sizeof(cache));

  for (uint8_t i = 0; i < MAX_CACHED_FAVORITE_TEAMS; i++) {
    if (cache.slots[i].team_index != settings.FavoriteTeam) continue;

    PersistedTeamData data = cache.slots[i];

    Team *fav = &TEAMS[settings.FavoriteTeam];
    fav->vs_id = data.vs_id;
    fav->score = data.score;
    fav->vs_score = data.vs_score;
    #ifndef PBL_PLATFORM_APLITE
    fav->ranking = data.ranking;
    fav->wins = data.wins;
    fav->postseasonGames = data.postseasonGames;
    fav->postseasonWins = data.postseasonWins;
    fav->postseasonLosses = data.postseasonLosses;
    #endif
    fav->completed = data.completed;
    fav->gametime = data.gametime;

    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Restored cached team data for FavoriteTeam %d (was cache slot %d): vsd=%d score=%d-%d completed=%d",
            settings.FavoriteTeam, i, fav->vs_id, fav->score, fav->vs_score, fav->completed);
    #endif

    // Re-promote to most-recently-used and persist the reordered cache.
    prv_team_cache_touch(&cache, data);
    persist_write_data(TEAM_DATA_KEY, &cache, sizeof(cache));
    return;
  }

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "FavoriteTeam %d not in the recently-used cache - forcing resync", settings.FavoriteTeam);
  #endif
  s_favorite_team_data_missing = true;
}

void globals_prv_update_display() {
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "-------- UPDATE DISPLAY --------");
  #endif

  // Only update if window exists
  if (!s_main_window) return;

  // API Check if empty
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "API Sync");
  #endif
  if (api_should_full_sync()) {
    api_request_cfbd_full_sync();
  }

  // Update beat_primary if DisplayTeam changed
  beat_primary = settings.DisplayTeam;

  // Update favorite team logo
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Favorite Team");
  #endif
  if (s_gbitmap_layers[GBITMAP_LAYER_LOGO]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_LOGO]);
  }
  #ifndef PBL_PLATFORM_APLITE
  if (s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]) {
    gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]);
  }
  #endif

  if (settings.hardcodeRival == 1){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - Rival");
    #endif
    settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
  }
  else if (settings.hardcodeRival == 2){
    if (TEAMS[settings.FavoriteTeam].vs_id == -1){
      if (settings.opponentSelect == 1){
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Rival)");
        #endif
        settings.BeatTeam = TEAMS[settings.FavoriteTeam].rival;
      }
      else if (settings.opponentSelect == 2){
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (Custom)");
        #endif
        settings.BeatTeam = settings.customOpponent;
      }
      else{
        #if defined(DEBUG)
        APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - BYE (NCAA)");
        #endif
        settings.BeatTeam = 1; //77
      }
    }
    else{
      #if defined(DEBUG)
      APP_LOG(APP_LOG_LEVEL_INFO, "Update Beat Team - API");
      #endif
      settings.BeatTeam = TEAMS[settings.FavoriteTeam].vs_id;
    }
  }

  // Resolve primary (displayed) and secondary team indices
  uint8_t primary_idx   = (settings.DisplayTeam > 1) ? settings.BeatTeam : settings.FavoriteTeam;
  #ifndef PBL_PLATFORM_APLITE
  uint8_t secondary_idx = (settings.DisplayTeam > 1) ? settings.FavoriteTeam : settings.BeatTeam;
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Display - %s", (settings.DisplayTeam > 1) ? "BEAT" : "Favorite");
  #endif

  // Main Window Background
  window_set_background_color(s_main_window, (GColor){.argb = TEAMS[primary_idx].color});

  s_gbitmap_layers[GBITMAP_LAYER_LOGO]      = gbitmap_create_with_resource(TEAMS[primary_idx].logo_res_id);
  #ifndef PBL_PLATFORM_APLITE
  s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM] = gbitmap_create_with_resource(TEAMS[secondary_idx].logo_res_id);

  // Setup Accent / Icon Colors
  GColor primary_icon_color = teams_get_icon_color(primary_idx);
  #endif
  
  // Update beat team layer position
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "BEAT Location");
  #endif
  if (s_layers[LAYER_BEAT_RECT]) {
    GRect new_frame = GRect(beat_spot, -40 + beat_primary, 44, 40);
    layer_set_frame(s_layers[LAYER_BEAT_RECT], new_frame);
    layer_mark_dirty(s_layers[LAYER_BEAT_RECT]);
  }
  
  #ifndef PBL_PLATFORM_APLITE
  display_setupBag(primary_icon_color);
  #endif

  #if defined(PBL_HEALTH)
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Health Colors");
  #endif
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_HR], primary_icon_color);
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_STEP], primary_icon_color);

  drawing_multiline_set_all_colors(hr_icon, primary_icon_color);
  drawing_multiline_set_all_colors(step_ladder, primary_icon_color);
  #endif

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Colors");
  #endif
  #ifndef PBL_PLATFORM_APLITE
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_WEATHER], primary_icon_color);
  text_layer_set_text_color(s_text_layers[TEXT_LAYER_CONDITIONS], primary_icon_color);
  #endif

  // 5. Update Secondary Badge Fill
  #ifndef PBL_PLATFORM_APLITE
  if (s_layers[LAYER_BEAT_TEAM]) {
    RoundRectData *beat_data = (RoundRectData *)layer_get_data(s_layers[LAYER_BEAT_TEAM]);
    if (beat_data) {
      beat_data->fill_color = (GColor){.argb = TEAMS[secondary_idx].color};
      layer_mark_dirty(s_layers[LAYER_BEAT_TEAM]);
    }
  }
  #endif

  if (s_bitmap_layers[BITMAP_LAYER_LOGO]) {
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_LOGO], s_gbitmap_layers[GBITMAP_LAYER_LOGO]);
  }
  #ifndef PBL_PLATFORM_APLITE
  if (s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM]) {
    bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_BEAT_TEAM], s_gbitmap_layers[GBITMAP_LAYER_BEAT_TEAM]);
  }
  #endif


  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather");
  #endif
  #ifndef PBL_PLATFORM_APLITE
  weather_update();
  #endif
  
  bool timeTrue = true;
  if (settings.countdownBool){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Countdown");
    #endif
    if (api_should_light_sync()) {
      api_request_cfbd_light_sync();
    }
    after_time = timekeeping_countdown();
    if ((!after_time  || !settings.scoreDisplayBool) && settings.countdownDisplay != 1){
      globals_what2show(s_day_text, s_hour_text, s_countdown_text, false, true);
      timeTrue = false;
    }
  }

  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_DEBUG, "After Time: %d", after_time);
  #endif
  if (settings.scoreDisplayBool && (!settings.countdownBool || (settings.countdownBool && after_time))){
    #if defined(DEBUG)
    APP_LOG(APP_LOG_LEVEL_INFO, "Update Score");
    #endif
    //OK This is the only time I'm not confident in after_time.  I also need to make a new variable for when it's checked the gametime since I should have that information.
    if (api_should_light_sync() && after_time) {
      api_request_cfbd_light_sync();
    }
    api_score_display();
    if (settings.scoreLocation != 1){
      // Score active: HOME/AWAY visible; TEXT_LAYER_TIME already holds
      // the score text from api_score_display() above.
      globals_what2show(s_home_text, s_away_text, s_score_text, false, false);
      timeTrue = false;
    }
  }

  if (timeTrue){
    globals_what2show("", "", s_time_text, true, true);
  }

  #ifndef PBL_PLATFORM_APLITE
  if (settings.rankingBool){
    if (TEAMS[settings.FavoriteTeam].ranking <= 25 && TEAMS[settings.FavoriteTeam].ranking > 0){

      static char s_rank_buffer[3];
      snprintf(s_rank_buffer, sizeof(s_rank_buffer), "#%d", TEAMS[settings.FavoriteTeam].ranking);
      text_layer_set_text(s_text_layers[TEXT_LAYER_RANK], s_rank_buffer);

      layer_set_hidden(s_layers[LAYER_RANK_RECT], false);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_RANK]), false);
    }
    else{
      layer_set_hidden(s_layers[LAYER_RANK_RECT], true);
      layer_set_hidden(text_layer_get_layer(s_text_layers[TEXT_LAYER_RANK]), true);
    }
  }

  if (settings.winBool){
    if(TEAMS[settings.FavoriteTeam].wins > 6){
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), false);
    }
    else{
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), true);
    }
    #ifdef TESTING
    layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_WIN]), false);
    #endif
  }


  if (settings.bowlBool){
    uint32_t target_res_id = 0;
    if(TEAMS[settings.FavoriteTeam].postseasonGames >= 1 && TEAMS[settings.FavoriteTeam].postseasonWins == 1){
      target_res_id = RESOURCE_ID_BOWL;
    }
    else if (TEAMS[settings.FavoriteTeam].postseasonLosses < 1 && TEAMS[settings.FavoriteTeam].postseasonWins == 3){
      target_res_id = RESOURCE_ID_CHAMP;
    }
    
    #ifdef TESTING
    target_res_id = RESOURCE_ID_CHAMP;
    #endif

    // Single Pass UI Update
    if (target_res_id != 0) {
      if (s_gbitmap_layers[GBITMAP_LAYER_TROPHY]) {
        gbitmap_destroy(s_gbitmap_layers[GBITMAP_LAYER_TROPHY]);
      }
      s_gbitmap_layers[GBITMAP_LAYER_TROPHY] = gbitmap_create_with_resource(target_res_id);
      bitmap_layer_set_bitmap(s_bitmap_layers[BITMAP_LAYER_TROPHY], s_gbitmap_layers[GBITMAP_LAYER_TROPHY]);
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), false);
    } else {
      layer_set_hidden(bitmap_layer_get_layer(s_bitmap_layers[BITMAP_LAYER_TROPHY]), true);
    }
  }
  #endif

  #if defined(PBL_HEALTH)
  #if defined(DEBUG)
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Health");
  #endif
  health_handler();
  #endif
}